#pragma once
#include <core/core.hpp>
#include <core/logging/logging.hpp>
#include <rendering/components/renderable.hpp>

using namespace legion::rendering;
using namespace legion::core::filesystem::literals;

class LightManager final : public System<LightManager>
{
public:
    static material_handle directionalLightMH;
    static model_handle directionalLightH;
    LightManager() : System<LightManager>()
    {

    }

    virtual void setup()
    {
        createProcess<&LightManager::update>("Update");
    }

    void update(time::span deltaTime)
    {

    }

    /**@brief Creates a Directional light
     * @returns entity_handle The entity_handle of the createdLight
     */
    static ecs::entity_handle createLight()
    {
        auto colorshader = rendering::ShaderCache::create_shader("color", "assets://shaders/color.shs"_view);

        directionalLightMH = rendering::MaterialCache::create_material("directional light", colorshader);
        directionalLightMH.set_param("color", math::color(1, 1, 0.8f));

        directionalLightH = rendering::ModelCache::create_model("directional light", "assets://models/directional-light.obj"_view);

        auto light = m_ecs->createEntity();
        light.add_component<rendering::light>(rendering::light::directional(math::color(1, 0, 0), 10.f));
        light.add_components<transform>(position(), rotation(rotation::lookat(math::vec3(1, 1, 1), math::vec3::zero)), scale());
        light.add_components<mesh_renderable>(mesh_filter(directionalLightH.get_mesh()), mesh_renderer(directionalLightMH));
        return light;
    }

    /**@brief Creates a Directional light
     * @param rot The direction the light should be facing.
     * @returns entity_handle The entity_handle of the createdLight
     */
    static ecs::entity_handle createLight(math::quat rot)
    {
        auto light = m_ecs->createEntity();
        light.add_component<rendering::light>(rendering::light::directional(math::color(1, 1, 0.8f), 10.f));
        light.add_components<transform>(position(), rotation(rot), scale());
        return light;
    }

    /**@brief Creates a Directional light
     * @param color The color of the light.
     * @param intensity The intensity of the light.
     * @returns entity_handle The entity_handle of the createdLight
     */
    static ecs::entity_handle createLight(math::color color, float intensity, math::quat rot)
    {
        auto light = m_ecs->createEntity();
        light.add_component<rendering::light>(rendering::light::directional(color, intensity));
        light.add_components<transform>(position(), rotation(rotation::lookat(math::vec3(1, 1, 1), math::vec3::zero)), scale());
        return light;
    }

    /**@brief Creates a light
     * @param type The type of light that wants be created.
     * @param pos The position you want to create the light at.
     * @param rot The direction the light should be facing.
     * @param s The size of the light, not sure if this matters.
     * @returns entity_handle The entity_handle of the createdLight
     */
    static ecs::entity_handle createLight(rendering::light type, math::vec3 pos, math::quat rot, math::vec3 s)
    {
        auto light = m_ecs->createEntity();
        light.add_component<rendering::light>(type);
        light.add_components<transform>(position(pos), rotation(rot), scale(s));
        return light;
    }

    /**@brief Creates a Directional Light
     * @param mesh The mesh you wish to create the light with.
     * @param material The material of the previous spoken of mesh.
     * @returns entity_handle The entity_handle of the createdLight
     */
    static ecs::entity_handle createLight(mesh_filter mesh, mesh_renderer material)
    {
        auto light = m_ecs->createEntity();
        light.add_components<rendering::mesh_renderable>(mesh, material);
        light.add_component<rendering::light>(rendering::light::directional(math::color(1, 1, 0.8f), 10.f));
        light.add_components<transform>(position(20, 20, 20), rotation::lookat(math::vec3(1, 1, 1), math::vec3::zero), scale());
        return light;
    }

    /**@brief Creates a light
     * @param mesh The mesh you wish to create the light with.
     * @param material The material of the previous spoken of mesh.
     * @param type The type of light that wants be created.
     * @returns entity_handle The entity_handle of the createdLight
     */
    static ecs::entity_handle createLight(mesh_filter mesh, mesh_renderer material, rendering::light type)
    {
        auto light = m_ecs->createEntity();
        light.add_components<rendering::mesh_renderable>(mesh, material);
        light.add_component<rendering::light>(type);
        light.add_components<transform>(position(20, 20, 20), rotation::lookat(math::vec3(1, 1, 1), math::vec3::zero), scale());
        return light;
    }

    /**@brief Creates a light
     * @param mesh The mesh you wish to create the light with.
     * @param material The material of the previous spoken of mesh.
     * @param type The type of light that wants be created.
     * @param pos The position you want to create the light at.
     * @param rot The direction the light should be facing.
     * @param s The size of the light, not sure if this matters.
     * @returns entity_handle The entity_handle of the createdLight
     */
    static ecs::entity_handle createLight(mesh_filter mesh, mesh_renderer material, rendering::light type, math::vec3 pos, math::quat rot, math::vec3 s)
    {
        auto light = m_ecs->createEntity();
        light.add_components<rendering::mesh_renderable>(mesh, material);
        light.add_component<rendering::light>(type);
        light.add_components<transform>(position(pos), rotation(rot), scale(s));
        return light;
    }

};

