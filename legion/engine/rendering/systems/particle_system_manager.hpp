#pragma once
#include <core/core.hpp>
#include <rendering/data/particle_system_base.hpp>
#include <rendering/components/point_emitter_data.hpp>
#include <core/compute/context.hpp>
#include <core/compute/kernel.hpp>

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
            log::debug("dt {}ms", deltaTime.milliseconds());
            OPTICK_EVENT();
            static auto emitters = createQuery<particle_emitter>();
            emitters.queryEntities();

            for (auto entity : emitters)
            {
                //Gets emitter handle and emitter.
                auto emitterHandle = entity.get_component_handle<particle_emitter>();
                auto emitter = emitterHandle.read();
                //Checks if emitter was already initialized.
                if (!emitter.setupCompleted)
                {
                    //If NOT then it goes through the particle system setup.
                    emitter.setupCompleted = true;
                    emitterHandle.write(emitter);

                    const ParticleSystemBase* particleSystem = emitter.particleSystemHandle.get();
                    particleSystem->setup(emitterHandle);
                }
                //else
                //{
                //    //If it IS then it runs the emitter through the particle system update.
                //    const ParticleSystemBase* particleSystem = emit.particleSystemHandle.get();
                //    particleSystem->update(emit.livingParticles, emitterHandle, emitters, deltaTime);
                //}

                const ParticleSystemBase* particleSystem = emitter.particleSystemHandle.get();

                auto dataHandle = entity.get_component_handle<rendering::point_emitter_data>();
                if (dataHandle && emitter.container)
                {
                    auto data = dataHandle.read();


                    auto camQuery = createQuery<camera>();
                    camQuery.queryEntities();
                    auto camPos = camQuery[0].get_component_handle<position>().read();

                    math::vec4 camPos4 = math::vec4(camPos.xyz, 0);
                    auto positionBuffer = compute::Context::createBuffer(emitter.container->positionBufferData, compute::buffer_type::READ_BUFFER, "positions");
                    auto colors = compute::Context::createBuffer(emitter.container->colorBufferData, compute::buffer_type::RW_BUFFER, "colors");
                    //auto outputColors = compute::Context::createBuffer(emitter.container->colorBufferData, compute::buffer_type::WRITE_BUFFER, "newColors");
                    int processSize = emitter.container->colorBufferData.size();
                    emitter.container->pointUpdateCL
                    (
                        processSize,
                        positionBuffer,
                        compute::karg(camPos4, "camPos"),
                        compute::karg(deltaTime, "deltaTime"),
                        colors
                    );

                    //schedule job to update animation 
                  /*  m_scheduler->queueJobs
                    (emitter.container->colorBufferData.size(), [&]()
                        {
                            auto value = async::this_job::get_id();
                            if (!emitter.container->isAnimating[value] && math::distance2(camPos, emitter.container->positionBufferData[value]) < 36.f)
                            {
                                emitter.container->isAnimating[value] = true;
                            }
                            if (emitter.container->isAnimating[value])
                            {
                                emitter.container->colorBufferData[value].a += deltaTime;
                            }
                        }
                    ).wait();*/
                }
            }
        }



        float getDistance(const math::vec3& camPos, const math::vec3& pointPos)
        {
            return math::distance2(camPos, pointPos);
        }
    };
}
