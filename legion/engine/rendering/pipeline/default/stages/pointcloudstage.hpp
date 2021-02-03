#pragma once
#include <rendering/pipeline/base/renderstage.hpp>
#include <rendering/pipeline/base/pipeline.hpp>
#include <rendering/components/point_cloud_particle_container.hpp>
#include <rendering/data/screen_quad.hpp>

namespace legion::rendering
{
    class PointCloudStage : public RenderStage<PointCloudStage>
    {
    private:
        shader_handle m_pointShader;
        shader_handle m_deferredLighting;

        texture_handle m_albedoBuffer;

        screen_quad m_screenquad;

        static point_cloud_particle_container* m_container;
    public:
        static void SetContainer(point_cloud_particle_container* container);
        void buffferCloud(point_cloud_particle_container& cloud);
        virtual void setup(app::window& context) override;
        virtual void render(app::window& context, camera& cam, const camera::camera_input& camInput, time::span deltaTime) override;
        virtual priority_type priority() override;
    };
}
