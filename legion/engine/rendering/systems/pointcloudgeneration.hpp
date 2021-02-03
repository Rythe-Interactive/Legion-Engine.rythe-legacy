#pragma once
#include<core/core.hpp>
#include <core/math/math.hpp>
#include <core/logging/logging.hpp>

#include <core/compute/context.hpp>
#include <core/compute/kernel.hpp>
#include <core/compute/high_level/function.hpp>

#include <rendering/systems/pointcloud_particlesystem.hpp>
#include <rendering/components/point_cloud.hpp>
#include <rendering/components/particle_emitter.hpp>
#include <rendering/components/lod.hpp>

#include <algorithm>
#include <execution>

using namespace legion;


namespace legion::rendering
{

    /**@class PointCloudGeneration
     * @brief A system that iterates all queried entities containing point_cloud and generates a particle system for them.
     */
    class PointCloudGeneration : public System<PointCloudGeneration>
    {
    public:

        /**@brief Setup inits the compute shader to sample the point clouds, creates update and does one initial generation.
          */
        void setup()
        {
            InitComputeShader();

            createProcess<&PointCloudGeneration::Update>("Rendering");

            Generate();
        }
        /**@Brief Every Update call Generate to check if new objects need to be generated
         */
        void Update(time::span deltaTime)
        {
            Generate();
        }

    private:
        //index to index the particle system name
        int cloudGenerationCount = 0;
        //query containing point clouds
        ecs::EntityQuery query = createQuery<point_cloud>();
        //compute shader
        compute::function pointCloudGeneratorCS;
        compute::function preProcessPointCloudCS;

        ParticleSystemHandle particleSystem;
        void InitComputeShader()
        {
            if (!pointCloudGeneratorCS.isValid())
                pointCloudGeneratorCS = fs::view("assets://kernels/pointRasterizer.cl").load_as<compute::function>("Main");
            if (!preProcessPointCloudCS.isValid())
                preProcessPointCloudCS = fs::view("assets://kernels/calculatePoints.cl").load_as<compute::function>("Main");
        }
        //query entities and iterate them
        void Generate()
        {
            query.queryEntities();
            for (auto& ent : query)
            {
                GeneratePointCloud(ent.get_component_handle<point_cloud>());
            }
        }
        //generates point clouds
        void GeneratePointCloud(ecs::component_handle<point_cloud> pointCloud)
        {
            using compute::in, compute::out, compute::karg;
            auto realPointCloud = pointCloud.read();

            //exit early if point cloud has already been generated
            if (realPointCloud.m_hasBeenGenerated) return;
            //read position
            math::mat4 transformMat = realPointCloud.m_trans.get_local_to_world_matrix();

            //generate particle params
            pointCloudParameters params{
                math::vec3(realPointCloud.m_pointRadius),
                realPointCloud.m_Material,
                ModelCache::get_handle("billboard")
            };

            std::vector<math::vec3> particleInput;
            std::vector<math::color> inputColor;
            std::vector<math::color> inputEmission;
            std::vector<math::vec3> inputNormals;

            int seed = math::linearRand<int>(std::numeric_limits<int>::min()+1, std::numeric_limits<int>::max());
            for (auto& meshHandle : realPointCloud.m_meshes)
            {
                //get mesh data
                auto m = meshHandle.get();
                auto vertices = m.second.vertices;
                auto indices = m.second.indices;
                auto uvs = m.second.uvs;
                uint triangle_count = indices.size() / 3;
                //triangle_count /= 16;
                log::debug("triangle count: " + std::to_string(triangle_count));
                //compute process size
                uint process_Size = triangle_count;

                //generate initial buffers from triangle info
                std::vector<uint> samplesPerTri(triangle_count);
                auto vertexBuffer = compute::Context::createBuffer(vertices, compute::buffer_type::READ_BUFFER, "vertices");
                auto indexBuffer = compute::Context::createBuffer(indices, compute::buffer_type::READ_BUFFER, "indices");
                uint totalSampleCount = 0;
                uint samplesPerTriangle = realPointCloud.m_maxPoints / triangle_count;
                //make sure samples per tri is at least 1
                if (samplesPerTriangle == 0) samplesPerTriangle = 1;

                ///PreProcess pointcloud
                //preprocess, calculate individual sample count per triangle
                std::vector<uint> output(triangle_count);
                {
                    auto outBuffer = compute::Context::createBuffer(output, compute::buffer_type::WRITE_BUFFER, "pointsCount");
                    auto computeResult = preProcessPointCloudCS
                    (
                        process_Size,
                        vertexBuffer,
                        indexBuffer,
                        karg(samplesPerTriangle, "samplesPerTri"),
                        outBuffer
                    );
                    //accumulate toutal triangle sample count
                    for (size_t i = 0; i < triangle_count; i++)
                    {
                        totalSampleCount += output.at(i);
                    }
                }

                ///Generate Point cloud
                //Generate points result vector
                std::vector<math::vec4> result(totalSampleCount);
                std::vector<math::color> resultColor(totalSampleCount);
                std::vector<math::color> resultEmission(totalSampleCount);
                std::vector<math::color> resultNormal(totalSampleCount);
                std::vector<math::vec4> lightDir(1);
                lightDir[0] = math::normalize(math::vec4(-1, -1, -1, 0));
                //Get normal map
                auto [lock, emission] = realPointCloud.m_emissionMap.get_raw_image();
                auto [lock2, albedo] = realPointCloud.m_AlbedoMap.get_raw_image();
                {
                    async::readonly_multiguard guard(lock, lock2);

                    auto emissionMapBuffer = compute::Context::createImage(emission, compute::buffer_type::READ_BUFFER, "emissionMap");

                    //Create buffers
                    auto albedoMapBuffer = compute::Context::createImage(albedo, compute::buffer_type::READ_BUFFER, "albedoMap");
                    auto sampleBuffer = compute::Context::createBuffer(output, compute::buffer_type::READ_BUFFER, "samples");
                    auto uvBuffer = compute::Context::createBuffer(uvs, compute::buffer_type::READ_BUFFER, "uvs");
                    auto lightDirBuffer = compute::Context::createBuffer(lightDir, compute::buffer_type::READ_BUFFER, "lightDir");

                    auto outBuffer = compute::Context::createBuffer(result, compute::buffer_type::WRITE_BUFFER, "points");
                    auto colorBuffer = compute::Context::createBuffer(resultColor, compute::buffer_type::WRITE_BUFFER, "colors");
                    auto emissionBuffer = compute::Context::createBuffer(resultEmission, compute::buffer_type::WRITE_BUFFER, "emission");
                    auto normalBuffer = compute::Context::createBuffer(resultNormal, compute::buffer_type::WRITE_BUFFER, "normals");
                    auto computeResult = pointCloudGeneratorCS
                    (
                        process_Size,
                        vertexBuffer,
                        indexBuffer,
                        uvBuffer,
                        sampleBuffer,
                        lightDirBuffer,
                        albedoMapBuffer,
                        emissionMapBuffer,
                        karg(seed, "seed"),
                        karg(process_Size, "triangleCount"),
                        outBuffer,
                        colorBuffer,
                        emissionBuffer,
                        normalBuffer
                    );
                }

                seed ^= math::linearRand<uint>(1, std::numeric_limits<uint>::max());

                size_type startIndex = particleInput.size();
                particleInput.resize(startIndex + result.size());
                auto it = particleInput.begin() + startIndex;
                std::transform(std::execution::par_unseq, result.begin(), result.end(), it, [&](math::vec4& item) { auto pos = transformMat * math::vec4(item.x, item.y, item.z, 1.f); return math::vec3(pos.x, pos.y, pos.z); });

                startIndex = inputNormals.size();
                inputNormals.resize(startIndex + resultNormal.size());
                it = inputNormals.begin() + startIndex;
                std::transform(std::execution::par_unseq, resultNormal.begin(), resultNormal.end(), it, [&](math::vec4& item) { auto normal = transformMat * math::vec4(item.x, item.y, item.z, 0.f); return math::vec3(normal.x, normal.y, normal.z); });

                inputColor.insert(inputColor.end(), resultColor.begin(), resultColor.end());
                inputEmission.insert(inputEmission.end(), resultEmission.begin(), resultEmission.end());
            }

            log::debug("Generated {} particles!", particleInput.size());

            GenerateParticles(params, particleInput, inputColor, inputEmission, inputNormals, realPointCloud.m_trans);


            //write that pc has been generated
            realPointCloud.m_hasBeenGenerated = true;
            pointCloud.write(realPointCloud);
        }

        void GenerateParticles(pointCloudParameters params,
            const std::vector<math::vec3>& input,
            const std::vector<math::color>& inputColor,
            const std::vector<math::color>& inputEmission,
            const std::vector<math::vec3>& inputNormals, transform trans)
        {
            //generate particle system
            std::string name = nameOfType<PointCloudParticleSystem>();



            auto newPointCloud = ParticleSystemCache::createParticleSystem<PointCloudParticleSystem>(name, params, input, inputColor, inputEmission, inputNormals);
            //create entity to store particle system
            auto newEnt = createEntity();

            //  newEnt.add_component <rendering::lod>();
            newEnt.add_components<transform>(trans.get<position>().read(), trans.get<rotation>().read(), trans.get<scale>().read());

            auto emitterHandle = newEnt.add_component<rendering::particle_emitter>();
            auto emitter = emitterHandle.read();
            emitter.particleSystemHandle = newPointCloud;
            newEnt.get_component_handle<rendering::particle_emitter>().write(emitter);
            //newPointCloud.get()->setup(emitterHandle, input, inputColor);

            //increment index
            cloudGenerationCount++;
        }
    };
}

