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

        cloud.colorBuffer = buffer(GL_ARRAY_BUFFER, cloud.colorBufferData, GL_STATIC_DRAW);
        cloud.vertexArray.setAttribPointer(cloud.colorBuffer, SV_COLOR, 4, GL_FLOAT, false, 0, 0);
        //cloud.vertexArray.setAttribDivisor(SV_COLOR, 1);

        cloud.emissionBuffer = buffer(GL_ARRAY_BUFFER, cloud.emissionBufferData, GL_STATIC_DRAW);
        cloud.vertexArray.setAttribPointer(cloud.emissionBuffer, 7, 4, GL_FLOAT, false, 0, 0);
        //cloud.vertexArray.setAttribDivisor(7, 1);

        cloud.positionBuffer = buffer(GL_ARRAY_BUFFER, cloud.positionBufferData, GL_STATIC_DRAW);
        cloud.vertexArray.setAttribPointer(cloud.positionBuffer, SV_POSITION, 3, GL_FLOAT, false, 0, 0);
        //cloud.vertexArray.setAttribDivisor(SV_POSITION, 1);        

        cloud.normalBuffer = buffer(GL_ARRAY_BUFFER, cloud.normalBufferData, GL_STATIC_DRAW);
        cloud.vertexArray.setAttribPointer(cloud.normalBuffer, SV_NORMAL, 3, GL_FLOAT, false, 0, 0);
        //cloud.vertexArray.setAttribDivisor(SV_NORMAL, 1);

        cloud.buffered = true;
    }

    void PointCloudStage::setup(app::window& context)
    {
        app::context_guard guard(context);
        m_pointShader = ShaderCache::create_shader("point cloud", fs::view("assets://shaders/point.shs"));
        m_deferredLighting = ShaderCache::create_shader("deferred lighting", fs::view("assets://shaders/pointclouddeferred.shs"));
        m_screenquad = screen_quad::generate();
    }

    void PointCloudStage::render(app::window& context, camera& cam, const camera::camera_input& camInput, time::span deltaTime)
    {
        static id_type mainId = nameHash("main");
        static id_type lightsId = nameHash("light buffer");
        static id_type lightCountId = nameHash("light count");

        if (!m_container)
            return;

        if (m_container->positionBufferData.empty())
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

        buffer* lightsBuffer = get_meta<buffer>(lightsId);

        size_type* lightCount = get_meta<size_type>(lightCountId);

        texture_handle sceneColor;
        auto colorAttachment = fbo->getAttachment(FRAGMENT_ATTACHMENT);
        if (std::holds_alternative<texture_handle>(colorAttachment))
            sceneColor = std::get<texture_handle>(colorAttachment);

        /*if (!m_albedoBuffer)
        {
            m_albedoBuffer = TextureCache::create_texture("pc albedo buffer", sceneColor.get_texture().size());
        }
        else
        {
            auto albedoTex = m_albedoBuffer.get_texture();
            auto sceneTex = sceneColor.get_texture();

            auto fboSize = sceneTex.size();

            if (albedoTex.size() != fboSize)
                albedoTex.resize(fboSize);
        }*/

        texture_handle sceneNormal;
        auto normalAttachment = fbo->getAttachment(NORMAL_ATTACHMENT);
        if (std::holds_alternative<texture_handle>(normalAttachment))
            sceneNormal = std::get<texture_handle>(normalAttachment);

        texture_handle scenePosition;
        auto positionAttachment = fbo->getAttachment(POSITION_ATTACHMENT);
        if (std::holds_alternative<texture_handle>(positionAttachment))
            scenePosition = std::get<texture_handle>(positionAttachment);

        if (!sceneColor || !sceneNormal || !scenePosition || !lightsBuffer || !lightCount)
            return;

        //fbo->attach(m_albedoBuffer, FRAGMENT_ATTACHMENT);
        fbo->bind();


       /* uint attachment = FRAGMENT_ATTACHMENT;
        glDrawBuffers(1, &attachment);

        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);

        uint attachments[4] = { FRAGMENT_ATTACHMENT, NORMAL_ATTACHMENT, POSITION_ATTACHMENT, OVERDRAW_ATTACHMENT };
        glDrawBuffers(4, attachments);*/

        m_pointShader.bind();
        m_pointShader.get_uniform<float>("size").set_value(0.05f);
        m_pointShader.get_uniform_with_location<math::mat4>(SV_VIEW).set_value(camInput.view);
        m_pointShader.get_uniform_with_location<math::mat4>(SV_PROJECT).set_value(camInput.proj);

        auto& cloud = *m_container;
        if (!cloud.buffered)
            buffferCloud(cloud);
        /*  else
          {
              cloud.colorBuffer.bufferData(cloud.colorBufferData);
          }*/

        cloud.vertexArray.bind();

        //glDrawArraysInstanced(GL_POINTS, 0, 1, cloud.positionBufferData.size());

        glDrawArrays(GL_POINTS, 0, cloud.positionBufferData.size());

        cloud.vertexArray.release();
        //fbo->release();

        //fbo->attach(sceneColor, FRAGMENT_ATTACHMENT);

        //fbo->bind();
        // lighting
        m_deferredLighting.bind();
        //m_deferredLighting.get_uniform_with_location<math::mat4>(SV_VIEW).set_value(camInput.view);
        //m_deferredLighting.get_uniform_with_location<math::mat4>(SV_PROJECT).set_value(camInput.proj);
        m_deferredLighting.get_uniform_with_location<math::vec4>(SV_CAMPOS).set_value(camInput.posnearz);
        m_deferredLighting.get_uniform<math::vec4>("skycolor").set_value(cam.clearColor);
        //m_deferredLighting.get_uniform_with_location<math::ivec2>(SV_VIEWPORT).set_value(camInput.viewportSize);

        m_deferredLighting.get_uniform_with_location<texture_handle>(SV_SCENECOLOR).set_value(sceneColor);
        m_deferredLighting.get_uniform_with_location<texture_handle>(SV_SCENEPOSITION).set_value(scenePosition);
        m_deferredLighting.get_uniform_with_location<texture_handle>(SV_SCENENORMAL).set_value(sceneNormal);
        m_deferredLighting.get_uniform_with_location<uint>(SV_LIGHTCOUNT).set_value(*lightCount);

        lightsBuffer->bind();
        m_screenquad.render();
        lightsBuffer->release();

        m_deferredLighting.release();

        fbo->release();
    }

    priority_type PointCloudStage::priority()
    {
        return opaque_priority;
    }
}
