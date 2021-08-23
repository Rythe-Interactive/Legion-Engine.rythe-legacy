#pragma once
#include <rendering/pipeline/base/pipeline.hpp>
#include <rendering/pipeline/default/defaultpipeline.hpp>

#include <unordered_map>

namespace legion::rendering
{
    class Renderer final : public System<Renderer>
    {
    private:
        static delegate<RenderPipelineBase*(app::window&)> m_pipelineProvider;
        std::atomic_bool m_exiting = false;

        static void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, L_MAYBEUNUSED const void* userParam);
        static void debugCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, L_MAYBEUNUSED const void* userParam);
        static void debugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, L_MAYBEUNUSED void* userParam);
        bool initContext(const app::window& window);

        void setThreadPriority();

        static RenderPipelineBase* m_currentPipeline;

    public:
        Renderer() : System<Renderer>()
        {
            setPipeline<DefaultPipeline>();
        }

        virtual void setup();

        void onExit(events::exit& event);

        void render(time::span deltatime);

        template<typename Pipeline, typename... Args CNDOXY(inherits_from<Pipeline, RenderPipeline<Pipeline>> = 0)>
        static void setPipeline(Args&&... args);

        L_NODISCARD static RenderPipelineBase* getPipeline(app::window& context);
        L_NODISCARD static RenderPipelineBase* getCurrentPipeline();
        L_NODISCARD static RenderPipelineBase* getMainPipeline();
    };
}

#include <rendering/systems/renderer.inl>
