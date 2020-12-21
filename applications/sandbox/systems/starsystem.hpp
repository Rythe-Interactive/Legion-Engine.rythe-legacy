#pragma once
#include <core/core.hpp>
#include <core/logging/logging.hpp>

using namespace legion::core::filesystem::literals;

class StarSystem final : public System<StarSystem>
{
public:

    StarSystem() : System<StarSystem>()
    {

    }
    virtual void setup()
    {
        /* rendering::model_handle sphereH = rendering::ModelCache::create_model("sphere", "assets://models/sphere.obj"_view);
         rendering::material_handle vertexColorH = rendering::MaterialCache::create_material("vertex color", "assets://shaders/vertexcolor.shs"_view);*/
        createProcess<&StarSystem::update>("Update");
    }

    void update(time::span deltaTime)
    {

    }

    static void generateStartSystem(math::vec3 pos)
    {
        uint32_t nLehmer = ((int)pos.x & 0xFFFF) << 16 | ((int)pos.z & 0xFFFF);
        generateStar(pos, rndInt(5, 10, nLehmer), math::color(rndFloat(0.0f, 1.0f, nLehmer), rndFloat(0.0f, 1.0f, nLehmer), rndFloat(0.0f, 1.0f, nLehmer), 1.0f),nLehmer);
    }

    static void generateStar(math::vec3 pos, int diamter, math::color starColor,uint32_t nLehmer)
    {
        rendering::model_handle sphereH = rendering::ModelCache::create_model("sphere", "assets://models/sphere.obj"_view);
        rendering::material_handle vertexColorH = rendering::MaterialCache::create_material("colorMat", "assets://shaders/vertexcolor.shs"_view);
        //vertexColorH.set_param("color", math::colors::blue);
        auto star = m_ecs->createEntity();
        star.add_components<transform>(position(pos), rotation(), scale(diamter / 2.0f));
        star.add_components<rendering::mesh_renderable>(mesh_filter(sphereH.get_mesh()), rendering::mesh_renderer(vertexColorH));
        generatePlanet(star, rndVec(-3,3,-3,3,-3,3,nLehmer));
    }

    static void generatePlanet(ecs::entity_handle star, math::vec3 posOffset)
    {
        auto pbrShader = rendering::ShaderCache::create_shader("pbr", "assets://shaders/pbr.shs"_view);
        rendering::model_handle sphereH = rendering::ModelCache::create_model("sphere", "assets://models/sphere.obj"_view);
        rendering::material_handle lavaH = rendering::MaterialCache::create_material("planet", pbrShader);
        lavaH.set_param("material_input.albedo", rendering::TextureCache::create_texture("assets://textures/lava/Stylized_Lava_Rocks_001_basecolor.jpg"_view));
        lavaH.set_param("material_input.normalHeight", rendering::TextureCache::create_texture("assets://textures/lava/Stylized_Lava_Rocks_001_normal.jpg"_view));
        lavaH.set_param("material_input.MRDAo", rendering::TextureCache::create_texture("assets://textures/lava/Stylized_Lava_Rocks_001_ambientOcclusion.jpg"_view));
        lavaH.set_param("material_input.emissive", rendering::TextureCache::create_texture("assets://textures/lava/Stylized_Lava_Rocks_001_basecolor.jpg"_view));
        lavaH.set_param("material_input.heightScale", 0.f);
        lavaH.set_param("discardExcess", false);
        lavaH.set_param("skycolor", math::color(0.1f, 0.3f, 1.0f));

        auto planetEnt = m_ecs->createEntity();
        transform t = star.get_component_handles<transform>();
        scale s = t.get<scale>().read();
        position p = t.get<position>().read();
        planetEnt.add_components<transform>(position(p.xyz + posOffset), rotation(), s.x / 2);
        planetEnt.add_components<rendering::mesh_renderable>(mesh_filter(sphereH.get_mesh()), rendering::mesh_renderer(lavaH));

    }

    static  uint32_t Lehmer32(uint32_t nLehmer)
    {
        nLehmer += 0xe120fc15;
        uint64_t tmp;
        tmp = (uint64_t)nLehmer * 0x4a39b70d;
        uint32_t m1 = (tmp >> 32) ^ tmp;
        tmp = (uint64_t)m1 * 0x12fad5c9;
        uint32_t m2 = (tmp >> 32) ^ tmp;
        return m2;
    }

    static int rndInt(int min, int max, uint32_t nLehmer)
    {
        return (Lehmer32(nLehmer) % (max - min)) + min;
    }

    static float rndFloat(float min, float max, uint32_t nLehmer)
    {
        return (Lehmer32(nLehmer) % (int)(max - min)) + min;
    }

    static math::vec3 rndVec(float minX, float maxX, float minY , float maxY, float minZ, float maxZ, uint32_t nLehmer)
    {
        float x = (Lehmer32(nLehmer) % (int)(maxX - minX)) + minX;
        float y = (Lehmer32(nLehmer) % (int)(maxY - minY)) + minY;
        float z = (Lehmer32(nLehmer) % (int)(maxZ - minZ)) + minZ;
        return math::vec3(x, y, z);
    }


};

struct star
{
    math::color color;
    std::vector<ecs::entity_handle> planets;
    star() = default;
};

struct planet
{
    planet() = default;
};
