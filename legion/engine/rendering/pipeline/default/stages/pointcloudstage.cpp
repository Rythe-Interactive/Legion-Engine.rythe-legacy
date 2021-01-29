#include <rendering/pipeline/default/stages/pointcloudstage.hpp>

namespace legion::rendering
{
    point_cloud_particle_container* PointCloudStage::m_container;

    void PointCloudStage::SetContainer(point_cloud_particle_container* container)
    {
        m_container = container;
    }

    void PointCloudStage::buffferCloud(point_cloud_particle_container& cloud)
    {
        cloud.vertexArray = vertexarray::generate();

        cloud.colorBuffer = buffer(GL_ARRAY_BUFFER, cloud.colorBufferData, GL_DYNAMIC_DRAW);
        cloud.vertexArray.setAttribPointer(cloud.colorBuffer, SV_COLOR, 4, GL_FLOAT, false, 0, 0);
        cloud.vertexArray.setAttribDivisor(SV_COLOR, 1);

        cloud.positionBuffer = buffer(GL_ARRAY_BUFFER, cloud.positionBufferData, GL_STATIC_DRAW);
        cloud.vertexArray.setAttribPointer(cloud.positionBuffer, SV_POSITION, 3, GL_FLOAT, false, 0, 0);
        cloud.vertexArray.setAttribDivisor(SV_POSITION, 1);

        cloud.buffered = true;
    }

    void PointCloudStage::setup(app::window& context)
    {
        app::context_guard guard(context);
        m_pointShader = ShaderCache::create_shader("point cloud", fs::view("assets://shaders/point.shs"));
    }

    void PointCloudStage::render(app::window& context, camera& cam, const camera::camera_input& camInput, time::span deltaTime)
    {
        static id_type mainId = nameHash("main");

        if (!m_container)
            return;

        auto* fbo = getFramebuffer(mainId);
        if (!fbo)
        {
            log::error("Main frame buffer is missing.");
            abort();
            return;
        }

        app::context_guard guard(context);
        if (!guard.contextIsValid())
        {
            abort();
            return;
        }

        auto [valid, message] = fbo->verify();
        if (!valid)
        {
            log::error("Main frame buffer isn't complete: {}", message);
            abort();
            return;
        }


        fbo->bind();
        m_pointShader.bind();
        m_pointShader.get_uniform<float>("size").set_value(0.05f);
        m_pointShader.get_uniform<math::vec4>("skycolor").set_value(math::vec4(0.005f, 0.0055f, 0.0065f, 1.0f));
        m_pointShader.get_uniform_with_location<math::mat4>(SV_VIEW).set_value(camInput.view);
        m_pointShader.get_uniform_with_location<math::mat4>(SV_PROJECT).set_value(camInput.proj);

        auto& cloud = *m_container;
        if (!cloud.buffered)
            buffferCloud(cloud);
        else
        {
            cloud.colorBuffer.bufferData(cloud.colorBufferData);
        }

        cloud.vertexArray.bind();

        glDrawArraysInstanced(GL_POINTS, 0, 1, cloud.positionBufferData.size());

        //glDrawArrays(GL_POINTS, 0, cloud.positionBufferData.size());

        cloud.vertexArray.release();

        m_pointShader.release();
        fbo->release();
    }

    priority_type PointCloudStage::priority()
    {
        return opaque_priority;
    }
}
