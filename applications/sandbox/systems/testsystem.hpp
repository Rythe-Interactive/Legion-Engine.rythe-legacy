#pragma once
#include <core/core.hpp>
#include <physics/halfedgeedge.hpp>
#include <application/application.hpp>
#include <core/math/math.hpp>

#include <core/logging/logging.hpp>
#include <physics/components/physics_component.hpp>
#include <physics/components/rigidbody.hpp>
#include <physics/cube_collider_params.hpp>
#include <physics/data/physics_manifold_precursor.h>
#include <physics/systems/physicssystem.hpp>
#include <physics/halfedgeface.hpp>
#include <physics/data/penetrationquery.h>


#include <core/compute/context.hpp>
#include <core/compute/kernel.hpp>
#include <core/compute/high_level/function.hpp>
#include <rendering/debugrendering.hpp>

#include <physics/physics_statics.hpp>
#include <physics/data/identifier.hpp>
#include <audio/audio.hpp>
#include <rendering/components/renderable.hpp>
#include <Voro++/voro++.hh>
#include <Voro++/common.hh>

#include <rendering/pipeline/default/stages/postprocessingstage.hpp>

#include "../data/animation.hpp"

using namespace legion;



struct sah
{
    int value;

    sah operator+(const sah& other)
    {
        return { value + other.value };
    }

    sah operator*(const sah& other)
    {
        return { value * other.value };
    }
};

//struct addRB{
//    math::vec3 force = math::vec3(0, 0, 30);
//    float time = 0.0f;
//    float addTime = 5.0f;
//    bool rigidbodyAdded = false;
//};
// Move and strive for the wireframe sphere (which holds the only audio source)
// For testing the movement of audio sources (Spatial audio/doppler)
struct audio_move : public app::input_axis<audio_move> {};
struct audio_strive : public app::input_axis<audio_strive> {};
// Gain and Pitch knob
struct gain_change : public app::input_axis<gain_change> {};
struct pitch_change : public app::input_axis<pitch_change> {};
struct play_audio_source : public app::input_action<play_audio_source> {};
struct pause_audio_source : public app::input_action<pause_audio_source> {};
struct stop_audio_source : public app::input_action<stop_audio_source> {};
struct rewind_audio_source : public app::input_action<rewind_audio_source> {};
struct audio_test_input : public app::input_action<audio_test_input> {};

struct nextEdge_action : public  app::input_action<nextEdge_action> {};
struct nextPairing_action : public  app::input_action<nextPairing_action> {};
//some test stuff just so i can change things

struct physics_test_move : public app::input_axis<physics_test_move> {};

struct light_switch : public app::input_action<light_switch> {};
struct tonemap_switch : public app::input_action<tonemap_switch> {};


struct activate_CRtest2 : public app::input_action<activate_CRtest2> {};
struct activate_CRtest3 : public app::input_action<activate_CRtest3> {};

struct activateFrictionTest : public app::input_action<activateFrictionTest> {};
//
//struct extendedPhysicsContinue : public app::input_action<extendedPhysicsContinue> {};
//struct nextPhysicsTimeStepContinue : public app::input_action<nextPhysicsTimeStepContinue> {};

using namespace legion::core::filesystem::literals;

class TestSystem final : public System<TestSystem>
{
public:
    ecs::entity_handle audioSphereLeft;
    ecs::entity_handle audioSphereRight;

    ecs::entity_handle sun;
    rendering::material_handle pbrH;
    rendering::material_handle copperH;
    rendering::material_handle aluminumH;


    virtual void setup()
    {
        physics::PrimitiveMesh::SetECSRegistry(m_ecs);

#pragma region Input binding

        app::InputSystem::createBinding<light_switch>(app::inputmap::method::F);
        app::InputSystem::createBinding<tonemap_switch>(app::inputmap::method::G);


        bindToEvent<light_switch, &TestSystem::onLightSwitch>();
        bindToEvent<tonemap_switch, &TestSystem::onTonemapSwitch>();

#pragma endregion

#pragma region Model and material loading
        const float additionalLightIntensity = 0.5f;

        rendering::model_handle directionalLightH;
        rendering::model_handle spotLightH;
        rendering::model_handle pointLightH;
        rendering::model_handle cubeH;
        rendering::model_handle sphereH;
        rendering::model_handle suzanneH;
        rendering::model_handle gnomeH;
        rendering::model_handle uvsphereH;
        rendering::model_handle axesH;
        rendering::model_handle submeshtestH;
        rendering::model_handle planeH;
        rendering::model_handle floorH;
        rendering::model_handle magneticLowH;
        rendering::model_handle cylinderH;
        rendering::model_handle billboardH;

        rendering::material_handle wireframeH;
        rendering::material_handle vertexColorH;

        rendering::material_handle uvH;
        rendering::material_handle textureH;
        rendering::material_handle texture2H;
        rendering::material_handle directionalLightMH;
        rendering::material_handle spotLightMH;
        rendering::material_handle pointLightMH;
        rendering::material_handle gizmoMH;
        rendering::material_handle normalH;
        rendering::material_handle billboardMH;
        rendering::material_handle particleMH;
        rendering::material_handle fixedSizeBillboardMH;
        rendering::material_handle fixedSizeParticleMH;

        app::window window = m_ecs->world.get_component_handle<app::window>().read();
        rendering::material_handle floorMH;

        {
            std::lock_guard guard(*window.lock);
            app::ContextHelper::makeContextCurrent(window);

            directionalLightH = rendering::ModelCache::create_model("directional light", "assets://models/directional-light.obj"_view);
            cubeH = rendering::ModelCache::create_model("cube", "assets://models/cube.obj"_view);
            cylinderH = rendering::ModelCache::create_model("cylinder", "assets://models/cylinder.obj"_view);
            sphereH = rendering::ModelCache::create_model("sphere", "assets://models/sphere.obj"_view);

            wireframeH = rendering::MaterialCache::create_material("wireframe", "assets://shaders/wireframe.shs"_view);
            vertexColorH = rendering::MaterialCache::create_material("vertex color", "assets://shaders/vertexcolor.shs"_view);
            uvH = rendering::MaterialCache::create_material("uv", "assets://shaders/uv.shs"_view);

            auto lightshader = rendering::ShaderCache::create_shader("light", "assets://shaders/light.shs"_view);
            directionalLightMH = rendering::MaterialCache::create_material("directional light", lightshader);
            directionalLightMH.set_param("color", math::color(1, 1, 0.8f));
            directionalLightMH.set_param("intensity", 1.f);


            texture2H = rendering::MaterialCache::create_material("texture", "assets://shaders/texture.shs"_view);
            texture2H.set_param("_texture", rendering::TextureCache::create_texture("assets://textures/split-test.png"_view));

            auto pbrShader = rendering::ShaderCache::create_shader("pbr", "assets://shaders/pbr.shs"_view);
            pbrH = rendering::MaterialCache::create_material("pbr", pbrShader);
            pbrH.set_param(SV_ALBEDO, rendering::TextureCache::create_texture("engine://resources/default/albedo"_view));
            pbrH.set_param(SV_NORMALHEIGHT, rendering::TextureCache::create_texture("engine://resources/default/normalHeight"_view));
            pbrH.set_param(SV_MRDAO, rendering::TextureCache::create_texture("engine://resources/default/MRDAo"_view));
            pbrH.set_param(SV_EMISSIVE, rendering::TextureCache::create_texture("engine://resources/default/emissive"_view));
            pbrH.set_param(SV_HEIGHTSCALE, 1.f);
            pbrH.set_param("discardExcess", false);
            pbrH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));

            copperH = rendering::MaterialCache::create_material("copper", pbrShader);
            copperH.set_param(SV_ALBEDO, rendering::TextureCache::create_texture("assets://textures/copper/copper-albedo-512.png"_view));
            copperH.set_param(SV_NORMALHEIGHT, rendering::TextureCache::create_texture("assets://textures/copper/copper-normalHeight-512.png"_view));
            copperH.set_param(SV_MRDAO, rendering::TextureCache::create_texture("assets://textures/copper/copper-MRDAo-512.png"_view));
            copperH.set_param(SV_EMISSIVE, rendering::TextureCache::create_texture("assets://textures/copper/copper-emissive-512.png"_view));
            copperH.set_param(SV_HEIGHTSCALE, 1.f);
            copperH.set_param("discardExcess", false);
            copperH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));

            aluminumH = rendering::MaterialCache::create_material("aluminum", pbrShader);
            aluminumH.set_param(SV_ALBEDO, rendering::TextureCache::create_texture("assets://textures/aluminum/aluminum-albedo-512.png"_view));
            aluminumH.set_param(SV_NORMALHEIGHT, rendering::TextureCache::create_texture("assets://textures/aluminum/aluminum-normalHeight-512.png"_view));
            aluminumH.set_param(SV_MRDAO, rendering::TextureCache::create_texture("assets://textures/aluminum/aluminum-MRDAo-512.png"_view));
            aluminumH.set_param(SV_EMISSIVE, rendering::TextureCache::create_texture("assets://textures/aluminum/aluminum-emissive-512.png"_view));
            aluminumH.set_param(SV_HEIGHTSCALE, 1.f);
            aluminumH.set_param("discardExcess", false);
            aluminumH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));


            normalH = rendering::MaterialCache::create_material("normal", "assets://shaders/normal.shs"_view);
            normalH.set_param(SV_NORMALHEIGHT, rendering::TextureCache::create_texture("engine://resources/default/normalHeight"_view));

            app::ContextHelper::makeContextCurrent(nullptr);
        }
#pragma endregion

#pragma region Entities

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(planeH.get_mesh()), rendering::mesh_renderer(pbrH));
            ent.add_components<transform>(position(0, 0.01f, 10), rotation(), scale(10));
        }

        {
            sun = createEntity();
            sun.add_components<rendering::mesh_renderable>(mesh_filter(directionalLightH.get_mesh()), rendering::mesh_renderer(directionalLightMH));
            sun.add_component<rendering::light>(rendering::light::directional(math::color(1, 1, 0.8f), 10.f));
            sun.add_components<transform>(position(10, 10, 10), rotation::lookat(math::vec3(1, 1, 1), math::vec3::zero), scale());
        }

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(submeshtestH.get_mesh()), rendering::mesh_renderer(pbrH));
            ent.add_component<sah>({});
            ent.add_components<transform>(position(0, 10, 0), rotation(), scale());
        }

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(axesH.get_mesh()), rendering::mesh_renderer(vertexColorH));
            ent.add_components<transform>();
        }

        //position positions[1000];
        //for (int i = 0; i < 1000; i++)
        //{
        //    positions[i] = position(math::linearRand(math::vec3(-10, -21, -10), math::vec3(10, -1, 10)));
        //}

        //time::timer clock;
        //time::timer entityClock;
        //time::time_span<time64> entityTime;
        //for (int i = 0; i < 00; i++)
        //{
        //    auto ent = createEntity();
        //    ent.add_components<rendering::mesh_renderable>(mesh_filter(sphereH.get_mesh()), rendering::mesh_renderer(pbrH));
        //    ent.add_component<sah>({});
        //    entityClock.start();
        //    ent.add_components<transform>(position(math::linearRand(math::vec3(-10, -21, -10), math::vec3(10, -1, 10))), rotation(), scale());
        //    entityTime += entityClock.end();
        //}
        //auto elapsed = clock.elapsedTime();
        //log::debug("Making entities took {}ms", elapsed.milliseconds());
        //log::debug("Creating transforms took {}ms", entityTime.milliseconds());

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(cubeH.get_mesh()), rendering::mesh_renderer(pbrH));
            ent.add_component<sah>({});
            ent.add_components<transform>(position(5.1f, 9, 0), rotation(), scale());
        }

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(sphereH.get_mesh()), rendering::mesh_renderer(copperH));
            ent.add_component<sah>({});
            ent.add_components<transform>(position(0, 3, -5.1f), rotation(), scale());
        }

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(sphereH.get_mesh()), rendering::mesh_renderer(aluminumH));
            ent.add_component<sah>({});
            ent.add_components<transform>(position(0, 3, -8.f), rotation(), scale());
        }

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(uvsphereH.get_mesh()), rendering::mesh_renderer(copperH));
            ent.add_component<sah>({});
            ent.add_components<transform>(position(0, 3, -3.6f), rotation(), scale());
        }

        {
            auto ent = createEntity();
            ent.add_components<rendering::mesh_renderable>(mesh_filter(uvsphereH.get_mesh()), rendering::mesh_renderer(aluminumH));
            ent.add_component<sah>({});
            ent.add_components<transform>(position(0, 3, -6.5f), rotation(), scale());
        }

#pragma endregion
        createProcess<&TestSystem::update>("Update");
        createProcess<&TestSystem::drawInterval>("Update");
        createProcess<&TestSystem::physicsUpdate>("Physics", 0.02f);
    }

#pragma region input stuff
    void onLightSwitch(light_switch* action)
    {
        static bool on = true;

        static auto decalH = gfx::MaterialCache::get_material("decal");

        if (!action->value)
        {
            //auto light = sun.read_component<rendering::light>();
            if (on)
            {
                /*light.set_intensity(0.f);
                sun.write_component(light);*/

                if (sun)
                    sun.destroy();

                decalH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                pbrH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                copperH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                aluminumH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
               /* ironH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                slateH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                rockH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                rock2H.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                fabricH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                bogH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                paintH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));
                skyboxH.set_param("skycolor", math::color(0.0001f, 0.0005f, 0.0025f));*/
            }
            else
            {
                if (!sun)
                {
                    sun = createEntity();
                    sun.add_components<rendering::mesh_renderable>(
                        mesh_filter(MeshCache::get_handle("directional light")),
                        rendering::mesh_renderer(rendering::MaterialCache::get_material("directional light")));

                    sun.add_component<rendering::light>(rendering::light::directional(math::color(1, 1, 0.8f), 10.f));
                    sun.add_components<transform>(position(10, 10, 10), rotation::lookat(math::vec3(1, 1, 1), math::vec3::zero), scale());
                }

                decalH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                pbrH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                copperH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                aluminumH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
               /* ironH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                slateH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                rockH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                rock2H.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                fabricH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                bogH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                paintH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));
                skyboxH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));*/
            }
            on = !on;
        }
    }

    void onTonemapSwitch(tonemap_switch* action)
    {
        static gfx::tonemapping_type algorithm = gfx::tonemapping_type::aces;

        if (!action->value)
        {
            switch (algorithm)
            {
            case gfx::tonemapping_type::aces:
                gfx::Tonemapping::setAlgorithm(gfx::tonemapping_type::reinhard);
                algorithm = gfx::tonemapping_type::reinhard;
                log::debug("Reinhard tonemapping");
                break;
            case gfx::tonemapping_type::reinhard:
                gfx::Tonemapping::setAlgorithm(gfx::tonemapping_type::reinhard_jodie);
                algorithm = gfx::tonemapping_type::reinhard_jodie;
                log::debug("Reinhard Jodie tonemapping");
                break;
            case gfx::tonemapping_type::reinhard_jodie:
                gfx::Tonemapping::setAlgorithm(gfx::tonemapping_type::legion);
                algorithm = gfx::tonemapping_type::legion;
                log::debug("Legion tonemapping");
                break;
            case gfx::tonemapping_type::legion:
                gfx::Tonemapping::setAlgorithm(gfx::tonemapping_type::unreal3);
                algorithm = gfx::tonemapping_type::unreal3;
                log::debug("Unreal3 tonemapping");
                break;
            case gfx::tonemapping_type::unreal3:
                gfx::Tonemapping::setAlgorithm(gfx::tonemapping_type::aces);
                algorithm = gfx::tonemapping_type::aces;
                log::debug("ACES tonemapping");
                break;
            default:
                gfx::Tonemapping::setAlgorithm(gfx::tonemapping_type::legion);
                algorithm = gfx::tonemapping_type::legion;
                log::debug("Legion tonemapping");
                break;
            }
        }
    }
#pragma endregion
    void update(time::span deltaTime)
    {
        /*static auto sahQuery = createQuery<sah, rotation, position>();

        for (auto entity : sahQuery)
        {
            auto rot = entity.read_component<rotation>();

            rot *= math::angleAxis(math::deg2rad(45.f * deltaTime), math::vec3(0, 1, 0));

            entity.write_component(rot);

            auto pos = entity.read_component<position>();
            debug::drawLine(pos, pos + rot.forward(), math::colors::magenta);
        }*/

      /*  if (rotate && !physics::PhysicsSystem::IsPaused)
        {
            for (auto entity : physicsFrictionTestRotators)
            {
                auto rot = entity.read_component<rotation>();

                rot *= math::angleAxis(math::deg2rad(-20.f * deltaTime), math::vec3(0, 0, 1));

                entity.write_component(rot);
            }
        }*/
    }

    void physicsUpdate(time::span deltaTime)
    {
        //static ecs::EntityQuery halfEdgeQuery = createQuery<physics::MeshSplitter>();

        //halfEdgeQuery.queryEntities();
        ////log::debug("halfEdgeQuery.size() {} ", halfEdgeQuery.size());
        //for (auto entity : halfEdgeQuery)
        //{
        //    auto edgeFinderH = entity.get_component_handle<physics::MeshSplitter>();
        //    auto [posH, rotH, scaleH] = entity.get_component_handles<transform>();

        //    math::mat4 transform = math::compose(scaleH.read(), rotH.read(), posH.read());

        //    auto splitter = edgeFinderH.read();

        //    //auto edgePtr = splitter.edgeFinder.currentPtr;

        //    //math::vec3 worldPos = transform * math::vec4(edgePtr->position, 1);
        //    //math::vec3 worldNextPos = transform * math::vec4(edgePtr->nextEdge->position, 1);

        //    //debug::drawLine(worldPos, worldNextPos, math::colors::red, 1.0f, 0.0f, true);

        //    //debug::drawLine(worldPos, worldPos + math::vec3(0, 0.1f, 0), math::colors::green, 5.0f, 0.0f, true);
        //    //debug::drawLine(worldNextPos, worldNextPos + math::vec3(0, 0.1f, 0), math::colors::blue, 5.0f, 0.0f, true);

        //    auto getEdge = entity.get_component_handle<physics::identifier>();

        //    for (size_t i = 0; i < splitter.debugHelper.intersectionIslands.size(); i++)
        //    {
        //        auto maxColor = splitter.debugHelper.colors.size();
        //        math::color color = splitter.debugHelper.colors[i % maxColor];

        //        auto island = splitter.debugHelper.intersectionIslands.at(i);

        //        for (auto pos : island)
        //        {
        //            math::vec3 worldIntersect = transform * math::vec4(pos, 1);
        //            debug::drawLine(worldIntersect, worldIntersect + math::vec3(0, 0.1f, 0), color, 10.0f, 0.0f);
        //        }


        //    }

        //    /* for (auto intersectingPosition : edgeFinder.debugHelper.intersectionsPolygons)
        //     {
        //         math::vec3 worldIntersect = transform * math::vec4(intersectingPosition, 1);
        //         debug::drawLine(worldIntersect, worldIntersect + math::vec3(0, 0.1f, 0), math::colors::blue, 10.0f, 0.0f);
        //     }*/

        //    for (auto intersectingPosition : splitter.debugHelper.nonIntersectionPolygons)
        //    {
        //        math::vec3 worldIntersect = transform * math::vec4(intersectingPosition, 1);
        //        debug::drawLine(worldIntersect, worldIntersect + math::vec3(0, 0.1f, 0), math::colors::yellow, 10.0f, 0.0f);
        //    }


        //    //log::debug("Count boundary polygon {} ");
        //    for (auto polygon : splitter.meshPolygons)
        //    {
        //        int boundaryCount = 0;
        //        math::vec3 worldCentroid = transform * math::vec4(polygon->localCentroid, 1);

        //        for (auto edge : polygon->GetMeshEdges())
        //        {
        //            if (edge->isBoundary)
        //            {
        //                boundaryCount++;

        //                math::vec3 worldEdgePos = transform * math::vec4(edge->position, 1);
        //                math::vec3 worldEdgeNextPos = transform * math::vec4(edge->nextEdge->position, 1);

        //                math::vec3 edgeToCentroid = (worldCentroid - worldEdgePos) * 0.05f;
        //                math::vec3 nextEdgeToCentroid = (worldCentroid - worldEdgeNextPos) * 0.05f;

        //                debug::drawLine(worldEdgePos + edgeToCentroid
        //                    , worldEdgeNextPos + nextEdgeToCentroid, polygon->debugColor, 5.0f, 0.0f, false);
        //            }

        //        }
        //        /*              math::vec3 normalWorld = transform * math::vec4(polygon->localNormal, 0);
        //                      debug::drawLine(worldCentroid
        //                          , worldCentroid + (normalWorld), polygon->debugColor, 5.0f, 0.0f, false);*/

        //                          // log::debug("polygon boundaryCount {} ", boundaryCount);

        //    }

        //    auto& boundaryInfoList = splitter.debugHelper.boundaryEdgesForPolygon;

        //    /* debug::drawLine(splitter.debugHelper.cuttingSetting.first
        //         , splitter.debugHelper.cuttingSetting.first + (splitter.debugHelper.cuttingSetting.second) * 2.0f, math::colors::cyan, 5.0f, 0.0f, false);*/



        //    for (size_t i = 0; i < boundaryInfoList.size(); i++)
        //    {
        //        auto& boundaryInfo = boundaryInfoList[i];
        //        math::color color = boundaryInfo.drawColor;

        //        if (i != splitter.debugHelper.polygonToDisplay) { continue; }

        //        math::vec3 polygonNormalOffset = boundaryInfo.worldNormal * 0.01f;

        //        debug::drawLine(boundaryInfo.intersectionPoints.first
        //            , boundaryInfo.intersectionPoints.second, math::colors::magenta, 10.0f, 0.0f, false);

        //        for (int j = 0; j < boundaryInfo.boundaryEdges.size(); j++)
        //        {
        //            auto edge = boundaryInfo.boundaryEdges.at(j);

        //            math::vec3 worldEdgePos = transform * math::vec4(edge->position, 1);
        //            math::vec3 worldEdgeNextPos = transform * math::vec4(edge->nextEdge->position, 1);

        //            float interpolant = (float)j / boundaryInfo.boundaryEdges.size();

        //            debug::drawLine(worldEdgePos
        //                , worldEdgeNextPos, math::lerp(color, math::colors::black, interpolant), 10.0f, 0.0f, false);

        //        }

        //        math::vec3 basePos = boundaryInfo.base + polygonNormalOffset;
        //        debug::drawLine(basePos
        //            , boundaryInfo.base + math::vec3(0, 0.1f, 0) + polygonNormalOffset, math::colors::red, 10.0f, 0.0f, false);

        //        debug::drawLine(boundaryInfo.prevSupport + polygonNormalOffset
        //            , boundaryInfo.prevSupport + math::vec3(0, 0.1f, 0) + polygonNormalOffset, math::colors::green, 10.0f, 0.0f, false);

        //        debug::drawLine(boundaryInfo.nextSupport + polygonNormalOffset
        //            , boundaryInfo.nextSupport + math::vec3(0, 0.1f, 0) + polygonNormalOffset, math::colors::blue, 10.0f, 0.0f, false);

        //        debug::drawLine(boundaryInfo.intersectionEdge + polygonNormalOffset
        //            , boundaryInfo.intersectionEdge + math::vec3(0, 0.1f, 0) + polygonNormalOffset, math::colors::magenta, 10.0f, 0.0f, false);
        //    }

        //}
    }

    void differentInterval(time::span deltaTime)
    {
        static time::span buffer;
        static int frameCount;
        static time::span accumulated;

        buffer += deltaTime;
        accumulated += deltaTime;
        frameCount++;

        math::vec2 v;
        v.x = 10;
        v.y = 20;

        if (buffer > 1.f)
        {
            buffer -= 1.f;
            //std::cout << "This is a fixed interval!! " << (frameCount / accumulated) << "fps " << deltaTime.milliseconds() << "ms" << std::endl;
        }
    }

    void drawInterval(time::span deltaTime)
    {
        static auto physicsQuery = createQuery< physics::physicsComponent>();
        uint i = 0;

        float duration = 0.02f;

        for (auto penetration : physics::PhysicsSystem::penetrationQueries)
        {
            debug::drawLine(penetration->faceCentroid
                , penetration->faceCentroid + penetration->normal, math::vec4(1, 0, 1, 1), 15.0f, duration);
            auto x = 1;
        }


        //--------------------------------------- Draw contact points ---------------------------------------//



        for (int i = 0; i < physics::PhysicsSystem::contactPoints.size(); i++)
        {
            //ref is red
            //inc is blue

            auto& contact = physics::PhysicsSystem::contactPoints.at(i);

            debug::drawLine(contact.refRBCentroid
                , contact.RefWorldContact, math::vec4(1, 0, 0, 1), 5.0f, duration, true);

            debug::drawLine(contact.incRBCentroid
                , contact.IncWorldContact, math::vec4(0, 0, 1, 1), 5.0f, duration, true);

            debug::drawLine(contact.IncWorldContact
                , contact.IncWorldContact + math::vec3(0, 0.1f, 0), math::vec4(0.5, 0.5, 0.5, 1), 5.0f, duration, true);

            debug::drawLine(contact.refRBCentroid
                , contact.refRBCentroid + math::vec3(0, 0.1f, 0), math::vec4(0, 0, 0, 1), 5.0f, duration, true);

        }

        //--------------------------------------- Draw extreme points ---------------------------------------//

        i = 0;
        for (auto penetration : physics::PhysicsSystem::aPoint)
        {
            debug::drawLine(penetration
                , penetration + math::vec3(0, 0.2, 0), math::vec4(1, 0, 0, 1), 15.0f);

        }
        i = 0;
        for (auto penetration : physics::PhysicsSystem::bPoint)
        {
            debug::drawLine(penetration
                , penetration + math::vec3(0, 0.2, 0), math::vec4(0, 0, 1, 1), 15.0f);

        }

        physics::PhysicsSystem::contactPoints.clear();
        physics::PhysicsSystem::penetrationQueries.clear();
        physics::PhysicsSystem::aPoint.clear();
        physics::PhysicsSystem::bPoint.clear();

        physicsQuery.queryEntities();
        auto size = physicsQuery.size();
        //this is called so that i can draw stuff
        for (auto entity : physicsQuery)
        {
            auto rotationHandle = entity.get_component_handle<rotation>();
            auto positionHandle = entity.get_component_handle<position>();
            auto scaleHandle = entity.get_component_handle<scale>();
            auto physicsComponentHandle = entity.get_component_handle<physics::physicsComponent>();

            bool hasTransform = rotationHandle && positionHandle && scaleHandle;
            bool hasNecessaryComponentsForPhysicsManifold = hasTransform && physicsComponentHandle;

            if (hasNecessaryComponentsForPhysicsManifold)
            {
                auto rbColor = math::color(0.0, 0.5, 0, 1);
                auto statibBlockColor = math::color(0, 1, 0, 1);

                rotation rot = rotationHandle.read();
                position pos = positionHandle.read();
                scale scale = scaleHandle.read();

                auto usedColor = statibBlockColor;
                bool useDepth = false;

                if (entity.get_component_handle<physics::rigidbody>())
                {
                    usedColor = rbColor;
                }


                //assemble the local transform matrix of the entity
                math::mat4 localTransform;
                math::compose(localTransform, scale, rot, pos);

                auto physicsComponent = physicsComponentHandle.read();

                i = 0;
                for (auto physCollider : *physicsComponent.colliders)
                {
                    //--------------------------------- Draw Collider Outlines ---------------------------------------------//

                    for (auto face : physCollider->GetHalfEdgeFaces())
                    {
                        //face->forEachEdge(drawFunc);
                        physics::HalfEdgeEdge* initialEdge = face->startEdge;
                        physics::HalfEdgeEdge* currentEdge = face->startEdge;

                        math::vec3 faceStart = localTransform * math::vec4(face->centroid, 1);
                        math::vec3 faceEnd = faceStart + math::vec3((localTransform * math::vec4(face->normal, 0)));

                        //debug::drawLine(faceStart, faceEnd, math::colors::green, 5.0f);

                        if (!currentEdge) { return; }

                        do
                        {
                            physics::HalfEdgeEdge* edgeToExecuteOn = currentEdge;
                            currentEdge = currentEdge->nextEdge;

                            math::vec3 worldStart = localTransform * math::vec4(edgeToExecuteOn->edgePosition, 1);
                            math::vec3 worldEnd = localTransform * math::vec4(edgeToExecuteOn->nextEdge->edgePosition, 1);

                            debug::drawLine(worldStart, worldEnd, usedColor, 2.0f, 0.0f, useDepth);

                        } while (initialEdge != currentEdge && currentEdge != nullptr);
                    }
                }

            }

        }

        //FindClosestPointsToLineSegment unit test


        math::vec3 p1(5, -0.5, 0);
        math::vec3 p2(5, 0.5, 0);

        math::vec3 p3(6, 0, -0.5);
        math::vec3 p4(6, 0, 0.5);

        math::vec3 p1p2;
        math::vec3 p3p4;

        debug::drawLine(p1, p2, math::colors::red, 5.0f);
        debug::drawLine(p3, p4, math::colors::red, 5.0f);

        physics::PhysicsStatics::FindClosestPointsToLineSegment(p1, p2, p3, p4, p1p2, p3p4);

        debug::drawLine(p1p2, p3p4, math::colors::green, 5.0f);

        p1 = math::vec3(8, 0, 0);
        p2 = p1 + math::vec3(0, 1.0f, 0);

        p3 = math::vec3(10, 0, 0) + math::vec3(1.0f);
        p4 = p3 - math::vec3(1.0f);

        debug::drawLine(p1, p2, math::colors::red, 5.0f);
        debug::drawLine(p3, p4, math::colors::red, 5.0f);

        physics::PhysicsStatics::FindClosestPointsToLineSegment(p1, p2, p3, p4, p1p2, p3p4);

        debug::drawLine(p1p2, p3p4, math::colors::green, 5.0f);

    }
};
