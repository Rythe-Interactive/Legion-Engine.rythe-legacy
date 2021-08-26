#include <physics/mesh_splitter_utils/primitive_mesh.hpp>
#include <rendering/components/renderable.hpp>

namespace legion::physics
{
    int PrimitiveMesh::count = 0;

    PrimitiveMesh::PrimitiveMesh
    (ecs::entity pOriginalEntity, std::vector<std::shared_ptr<SplittablePolygon>>& pPolygons, rendering::material_handle pOriginalMaterial)
        : polygons(std::move(pPolygons)), originalMaterial(pOriginalMaterial), originalEntity(pOriginalEntity)
    {

    }

    ecs::entity PrimitiveMesh::InstantiateNewGameObject()
    {
        auto [originalPosH, originalRotH, originalScaleH] = originalEntity.get_component<position, rotation, scale>();
        math::mat4 trans = math::compose(originalScaleH.get(), originalRotH.get(), originalPosH.get());

        auto ent = ecs::Registry::createEntity();
        math::vec3 offset;

        mesh newMesh;

        math::vec3 scale = originalScaleH.get();
        populateMesh(newMesh, trans, offset, scale);

        newMesh.calculate_tangents(&newMesh);

        sub_mesh newSubMesh;
        newSubMesh.indexCount = newMesh.indices.size();
        newSubMesh.indexOffset = 0;

        newMesh.submeshes.push_back(newSubMesh);

        //creaate modelH
        mesh_handle meshH = core::MeshCache::create_mesh("newMesh" + std::to_string(count), newMesh);
        auto modelH = rendering::ModelCache::create_model(meshH);
        count++;

        //create renderable
        mesh_filter meshFilter = mesh_filter(meshH);

        ent.add_component<rendering::mesh_renderable>(meshFilter, rendering::mesh_renderer(originalMaterial));

        //create transform

        auto [posH, rotH, scaleH] = ecs::Registry::createComponent<transform>(ent);
        math::vec3 newEntityPos = originalPosH.get() + offset;
        posH = newEntityPos;
        rotH = originalRotH.get();
        //scaleH.write(originalScaleH.read());

        math::vec3 initialPos = originalPosH.get();
        //+ math::vec3(7, 0, 0)
        originalPosH = initialPos;
        //m_ecs->destroyEntity(originalEntity);

        return ent;
    }

    void PrimitiveMesh::populateMesh(mesh& mesh,
        const math::mat4& originalTransform, math::vec3& outOffset, math::vec3& scale)
    {
        std::vector<uint>& indices = mesh.indices;
        std::vector<math::vec3>& vertices = mesh.vertices;
        std::vector<math::vec2>& uvs = mesh.uvs;
        std::vector<math::vec3>& normals = mesh.normals;

        //for each polygon in splittable polygon

        for (auto polygon : polygons)
        {
            if (polygon->GetMeshEdges().empty())
            {
                log::error("Primitive Mesh has has an empty polygon");
                continue;
            }

            polygon->ResetEdgeVisited();

            std::queue<meshHalfEdgePtr> unvisitedEdgeQueue;
            unvisitedEdgeQueue.push(polygon->GetMeshEdges().at(0));

            while (!unvisitedEdgeQueue.empty())
            {
                auto halfEdge = unvisitedEdgeQueue.front();
                unvisitedEdgeQueue.pop();

                if (!halfEdge->isVisited)
                {
                    halfEdge->markTriangleEdgeVisited();

                    auto [edge1, edge2, edge3] = halfEdge->getTriangle();

                    vertices.push_back(edge1->position);
                    vertices.push_back(edge2->position);
                    vertices.push_back(edge3->position);

                    uvs.push_back(edge1->uv);
                    uvs.push_back(edge2->uv);
                    uvs.push_back(edge3->uv);

                    if (!edge1->isBoundary && edge1->pairingEdge)
                    {
                        unvisitedEdgeQueue.push(edge1->pairingEdge);
                    }

                    if (!edge2->isBoundary && edge2->pairingEdge)
                    {
                        unvisitedEdgeQueue.push(edge2->pairingEdge);
                    }

                    if (!edge3->isBoundary && edge3->pairingEdge)
                    {
                        unvisitedEdgeQueue.push(edge3->pairingEdge);
                    }
                }
            }
        }

        //get centroid of vertices
        math::vec3 worldCentroid = math::vec3();

        for (const auto& vertex : vertices)
        {
            worldCentroid += vertex;
        }

        worldCentroid /= static_cast<float>(vertices.size());

        worldCentroid = originalTransform * math::vec4(worldCentroid, 1);
        math::vec3 originalPosition = originalTransform[3];

        outOffset = worldCentroid - originalPosition;

        math::vec3 localOffset = math::inverse(originalTransform) * math::vec4(outOffset, 0);

        //shift vertices by offset
        for (auto& vertex : vertices)
        {
            vertex -= localOffset;
            vertex *= scale;
        }


        for (int i = 0; i < vertices.size(); i++)
        {
            indices.push_back(i);
        }

        for (int i = 0; i < vertices.size(); i += 3)
        {
            math::vec3& v1 = vertices.at(i);
            math::vec3& v2 = vertices.at(i + 1);
            math::vec3& v3 = vertices.at(i + 2);

            math::vec3 normal = math::cross(v2 - v1, v3 - v1);

            normals.push_back(normal);
        }
    }
}
