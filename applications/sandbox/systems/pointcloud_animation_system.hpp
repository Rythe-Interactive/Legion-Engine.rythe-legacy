#pragma once
#include<core/core.hpp>
#include<rendering/components/point_emitter_data.hpp>
#include<rendering/components/point_animation_data.hpp>
namespace legion
{
    class pointcloudanimationsystem final : public System<pointcloudanimationsystem>
    {
    public:

        void setup() override
        {
            createProcess<&pointcloudanimationsystem::update>("Update");
        }

        void update(time::span deltaTime)
        {
            UpdateCam();

            m_entityQuery.queryEntities();
            //log::debug(std::to_string(m_entityQuery.size()));
            for (auto entity : m_entityQuery)
            {
                auto dataHandle = entity.get_component_handle<point_animation_data>();
                auto data = dataHandle.read();
                Animate(entity, deltaTime, data, dataHandle);
            }
        }

    private:
        void Animate(ecs::entity_handle entityHanlde, float deltaTime, point_animation_data& data, ecs::component_handle< point_animation_data>& dataHandle)
        {
            auto posHandle = entityHanlde.get_component_handle<position>();
            math::vec3 pos = posHandle.read();
            //Check if animation has started, move if true
            if (data.isAnimating)
            {
                pos += math::vec3::down * m_speed * deltaTime;
            }
            else
            {
                //animation has not started, calculate distance and decide if you are close  enough to start the animation
                float distance = math::distance(m_camPos, pos);
                if (distance < m_threshold)
                {
                    //move and write that animation has started
                    pos += math::vec3::down * m_speed * deltaTime;
                    data.isAnimating = true;
                    dataHandle.write(data);
                }
            }

            posHandle.write(pos);

        }
        void UpdateCam()
        {
            m_CamQuery.queryEntities();
            for (ecs::entity_handle entity : m_CamQuery)
            {
                if (entity.has_component<position>())
                {
                    m_camPos = entity.get_component_handle<position>().read();
                }
            }
        }

        ecs::EntityQuery m_entityQuery = createQuery<point_animation_data>();
        ecs::EntityQuery m_CamQuery = createQuery<camera>();
        math::vec3 m_camPos;

        const float m_speed = 1.0f;
        const float m_threshold = 3.5f;
    };

}
