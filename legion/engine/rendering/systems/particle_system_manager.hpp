#pragma once
#include <core/core.hpp>
#include <rendering/data/particle_system_base.hpp>
#include <rendering/components/point_emitter_data.hpp>
namespace legion::rendering
{
    /**
     * @class ParticleSystemManager
     * @brief The class used to update all particles in every emitter.
     */
    class ParticleSystemManager : public System<ParticleSystemManager>
    {
    public:
        ParticleSystemManager()
        {
            ParticleSystemBase::m_registry = m_ecs;
        }
        /**
         * @brief Sets up the particle system manager.
         */
        void setup()
        {
            createProcess<&ParticleSystemManager::update>("Update");
        }
        /**
         * @brief Every frame, goes through every emitter and updates their particles with their respective particle systems.
         * @param deltaTime The delta time to be used inside of the update.
         */
        void update(time::span deltaTime)
        {
            OPTICK_EVENT();
            static auto emitters = createQuery<particle_emitter>();
            emitters.queryEntities();
            for (auto entity : emitters)
            {
                //Gets emitter handle and emitter.
                auto emitterHandle = entity.get_component_handle<particle_emitter>();
                auto emit = emitterHandle.read();
                //Checks if emitter was already initialized.
                if (!emit.setupCompleted)
                {
                    //If NOT then it goes through the particle system setup.
                    emit.setupCompleted = true;
                    emitterHandle.write(emit);

                    const ParticleSystemBase* particleSystem = emit.particleSystemHandle.get();
                    particleSystem->setup(emitterHandle);
                }
                else
                {
                    //If it IS then it runs the emitter through the particle system update.
                    const ParticleSystemBase* particleSystem = emit.particleSystemHandle.get();
                    particleSystem->update(emit.livingParticles, emitterHandle, emitters, deltaTime);
                }
            }
            //update point cloud buffer data
            static auto pointCloudQuery = createQuery<particle_emitter, rendering::point_emitter_data>();
            pointCloudQuery.queryEntities();
            std::vector<math::vec4> colorData;
            int index = 0;
            for (auto pointEntities : pointCloudQuery)
            {
                auto emitterHandle = pointEntities.get_component_handle<particle_emitter>();
                auto emitter = emitterHandle.read();
                const ParticleSystemBase* particleSystem = emitter.particleSystemHandle.get();

                auto dataHandle = pointEntities.get_component_handle<rendering::point_emitter_data>();
                auto data = dataHandle.read();

                index++;
                if (index == pointCloudQuery.size())
                {
                /*    auto windowHandle = world.get_component_handle<app::window>();
                    if (!windowHandle)return;*/

                    //get data
                    auto& colorData = emitter.container->colorBufferData;
                    auto& posData = emitter.container->colorBufferData;
                    auto& isAnimating = emitter.container->isAnimating;
                    //Get cam pos
                    auto camQuery = createQuery<camera>();
                    camQuery.queryEntities();
                    auto camPos = camQuery[0].get_component_handle<position>().read();

                    //schedule job to update animation 
                    m_scheduler->queueJobs
                    (colorData.size(), [&]()
                        {
                            auto value = async::this_job::get_id();
                            if (!isAnimating[value] && getDistance(camPos, posData[value]) < 16.0f)
                            {
                                isAnimating[value] = true;
                            }
                            if(isAnimating[value])
                            {
                                colorData[value].a += deltaTime;
                            }
                        }
                    ).wait();
                    ////update alpha based on position distance
                    //for (size_t i = 0; i < colorData.size(); i++)
                    //{
                    //    colorData.at(i).a = getDistance(posData.at(i), camPos);
                    //}

                    //app::context_guard guard(windowHandle.read());
                    //if (guard.contextIsValid())
                    //{

                    //    rendering::buffer colorBuffer = rendering::buffer(GL_ARRAY_BUFFER, emitter.container->colorBufferData, GL_STREAM_READ);
                    //    particleSystem->m_particleModel.overwrite_buffer(colorBuffer, SV_COLOR, true);
                    //}
                }
            }
        }



        float getDistance(const math::vec3& camPos, const math::vec3& pointPos)
        {
            return math::distance2(camPos, pointPos);
        }
    };
}
