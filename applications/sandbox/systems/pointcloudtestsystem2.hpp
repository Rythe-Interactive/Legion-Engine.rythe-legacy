#pragma once
#include<core/core.hpp>
#include <core/math/math.hpp>
#include <core/logging/logging.hpp>
#include <application/application.hpp>

#include <core/compute/context.hpp>
#include <core/compute/kernel.hpp>


#include <rendering/components/particle_emitter.hpp>

#include <rendering/systems/pointcloudgeneration.hpp>
using namespace legion;
using namespace rendering;
//system to test the point cloud generation system and the point cloud component
class pointcloudtestsystem2 final : public legion::core::System<pointcloudtestsystem2>
{
public:
    ecs::entity_handle player;

    struct player_move : public app::input_axis<player_move> {};
    struct player_strive : public app::input_axis<player_strive> {};
    struct player_fly : public app::input_axis<player_fly> {};
    struct player_look_x : public app::input_axis<player_look_x> {};
    struct player_look_y : public app::input_axis<player_look_y> {};

    virtual void setup() override
    {
        using namespace fs::literals;
        //get mesh
        //ModelCache::create_model("cube", "assets://models/Cube.obj"_view);
        //ModelCache::create_model("plane", "assets://models/plane.obj"_view);
        //ModelCache::create_model("billboard", "assets://models/billboard.obj"_view);

        //ModelCache::create_model("uvsphere", "assets://models/uvsphere.obj"_view);
        //ModelCache::create_model("sphere", "assets://models/sphere.obj"_view);
        //ModelCache::create_model("suzanne", "assets://models/suzanne.obj"_view);


        //create particle system material
        material_handle particleMaterial;
        material_handle billboardMat;
        rendering::material_handle rockH;
        //rendering::texture m_normalMap;
        rendering::texture_handle m_normalMap;
        image_handle normal;
        image_handle albedo;
        image_handle albedo2;
        image_handle albedo3;
        image_handle planeAlbedo;
        model_handle testHandle;
        material_handle testMat;
        std::vector<material_handle> materials_vector;

        app::window window = m_ecs->world.get_component_handle<app::window>().read();
        {
            app::context_guard guard(window);

            //get mesh
            //ModelCache::create_model("2s1", "assets://models/2story_3.glb"_view, materials_vector);
            //ModelCache::create_model("cube", "assets://models/Cube.obj"_view);
            ModelCache::create_model("plane", "assets://models/plane.obj"_view);
            //ModelCache::create_model("billboard", "assets://models/billboard.obj"_view);
            //ModelCache::create_model("sponza", "assets://models/sponza_structure.obj"_view);
            ModelCache::create_model("AirPlane", "assets://models/AirPlane.obj"_view);
            ModelCache::create_model("village", "assets://models/village_w_floor_v2.obj"_view);

            //ModelCache::create_model("uvsphere", "assets://models/uvsphere.obj"_view);
            //ModelCache::create_model("sphere", "assets://models/sphere.obj"_view);
            ModelCache::create_model("suzanne", "assets://models/suzanne.obj"_view);

            //auto colorshader = rendering::ShaderCache::create_shader("color", "assets://shaders/color.shs"_view);
            //auto uvShader = rendering::ShaderCache::create_shader("uv", "assets://shaders/uv.shs"_view);
            //testMat = rendering::MaterialCache::create_material("uvMat", "assets://shaders/uv.shs"_view);

            normal = ImageCache::create_image("normal image", "assets://textures/mcTexEmission.png"_view);
            albedo2 = ImageCache::create_image("albedo2 image", "assets://textures/mcTex.png"_view);

            testMat = rendering::MaterialCache::create_material("test mat", "assets://shaders/test.shs"_view);
            testMat.set_param("useAlbedoTex", true);
            testMat.set_param("alphaCutoff", 0.5f);
            testMat.set_param("albedoTex", TextureCache::create_texture_from_image(albedo2, {
        texture_type::two_dimensional, channel_format::eight_bit, texture_format::rgba,
        texture_components::rgba, true, true, texture_mipmap::nearest, texture_mipmap::nearest,
        texture_wrap::repeat, texture_wrap::repeat, texture_wrap::repeat }));
            testMat.set_param("useMetallicRoughness", false);
            testMat.set_param("useMetallicTex", false);
            testMat.set_param("metallicValue", 0.999f);
            testMat.set_param("useRoughnessTex", false);
            testMat.set_param("roughnessValue", 0.1f);
            testMat.set_param("useEmissiveTex", true);
            testMat.set_param("emissiveTex", TextureCache::create_texture_from_image(normal, {
        texture_type::two_dimensional, channel_format::eight_bit, texture_format::rgba,
        texture_components::rgba, true, true, texture_mipmap::nearest, texture_mipmap::nearest,
        texture_wrap::repeat, texture_wrap::repeat, texture_wrap::repeat }));
            testMat.set_param("useNormal", false);
            testMat.set_param("useAmbientOcclusion", false);
            testMat.set_param("useHeight", false);
            testMat.set_param("skycolor", math::color(0.1f, 0.3f, 1.f));

            auto billBoardsh = rendering::ShaderCache::create_shader("billboard", "assets://shaders/point.shs"_view);
            billboardMat = rendering::MaterialCache::create_material("billboardMat", billBoardsh);

            //particleMaterial = rendering::MaterialCache::create_material("directional light", colorshader);
            //particleMaterial.set_param("color", math::colors::blue);

            //albedo = ImageCache::create_image("albedo image", "assets://textures/blue.png"_view);
            //albedo2 = ImageCache::create_image("albedo2 image", "assets://textures/red.png"_view);

            //auto a = materials_vector.front();
            //auto params = a.get_params();
            //auto texture = a.get_param<texture_handle>("material_input.albedo");

            //auto path = texture.get_texture().path;
            //auto name = texture.get_texture().name;
            //const auto& tex = texture.get_data();
        }

        material_handle mat = MaterialCache::get_material("uv");
        mesh_handle uvMesh = MeshCache::get_handle("uvsphere");
        mesh_handle cubeMesh = MeshCache::get_handle("cube");
        mesh_handle sphereMesh = MeshCache::get_handle("sphere");
        mesh_handle suzanneeMesh = MeshCache::get_handle("suzanne");
        //mesh_handle sponzaMesh = MeshCache::get_handle("sponza");
        mesh_handle airPlaine = MeshCache::get_handle("AirPlane");
        mesh_handle plane = MeshCache::get_handle("plane");
        mesh_handle s21 = MeshCache::get_handle("2s1");
        mesh_handle village = MeshCache::get_handle("village");
        log::debug("material capacity " + std::to_string(materials_vector.capacity()));

        //for(auto [key, value] : params){}

        //mesh_handle CarnegieMansion = MeshCache::get_handle("CarnegieMansion");
        /*auto ent2 = createEntity();
        auto trans2 = ent2.add_components<transform>(position(0, 1, 5), rotation(), scale(0.5f));
        ent2.add_component<point_cloud>(point_cloud(sphereMesh, trans2, billboardMat, albedo, normal, 5000, 0.05f));*/

        /*auto ent = createEntity();
        auto trans = ent.add_components<transform>(position(0, 2, 15), rotation(), scale(0.5f));
        ent.add_component<point_cloud>(point_cloud(sphereMesh, trans, billboardMat, albedo3, normal, 1500, 0.05f));*/

        /*for(int i = 0; i < 8; i++)
        {
            auto ent = createEntity();
            auto trans = ent.add_components<transform>(position(0, 2, 4+i*2 ), rotation(), scale(0.5f));
            ent.add_component<point_cloud>(point_cloud(sphereMesh, trans, billboardMat, albedo, normal, 1500, 0.05f));

            auto ent2 = createEntity();
            auto trans2 = ent2.add_components<transform>(position(3.0f, 2, 4 + i*2), rotation(), scale(0.5f));
            ent2.add_component<point_cloud>(point_cloud(sphereMesh, trans2, billboardMat, albedo2, normal, 1500, 0.05f));
        }*/


        auto sun = createEntity();
        //sun.add_components<rendering::mesh_renderable>(mesh_filter(directionalLightH.get_mesh()), rendering::mesh_renderer(directionalLightMH));
        sun.add_component<rendering::light>(rendering::light::directional(math::color(1, 1, 0.8f), 10.f));
        sun.add_components<transform>(position(10, 10, 10), rotation::lookat(math::vec3(1, 1, 1), math::vec3::zero), scale());

        auto ent2 = createEntity();
        auto trans2 = ent2.add_components<transform>(position(0.f, -20.f, 0.f), rotation(), scale());
        //ent2.add_components<rendering::mesh_renderable>(mesh_filter(village), rendering::mesh_renderer(testMat));

       /* auto ent = createEntity();
        ent.add_components<transform>(position(0.f, -10.1f, 0.f), rotation(), scale(50.f));
        ent.add_components<rendering::mesh_renderable>(mesh_filter(plane), rendering::mesh_renderer(testMat));*/

        std::vector<mesh_handle> meshes{ village };

        ent2.add_component<point_cloud>(point_cloud(meshes, trans2, billboardMat, albedo2, normal, 1000000, 0.1f));


        /*auto ent4 = createEntity();
        ent4.add_components<transform>(position(-1.5f, 1, 0), rotation(), scale(0.5f));
        ent4.add_components<rendering::mesh_renderable>(mesh_filter(uvMesh), rendering::mesh_renderer(testMat));*/

        //ent1.add_component<point_cloud>(point_cloud(suzanneeMesh, trans1, billboardMat, albedo, normal, 10000, 0.025f));

        /*auto ent3 = createEntity();
        auto trans3 = ent3.add_components<transform>(position(-1.5f, 1, 0), rotation(), scale(0.5f));
        ent3.add_component<point_cloud>(point_cloud(cubeMesh, trans3, billboardMat, albedo, normal, 2000, 0.1f));*/

    }



};





