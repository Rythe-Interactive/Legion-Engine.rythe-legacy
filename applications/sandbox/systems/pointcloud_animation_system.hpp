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
            OPTICK_EVENT();
            UpdateCam();

            m_entityQuery.queryEntities();
            //bulk read
            auto& positions = m_entityQuery.get<position>();
            auto& animData = m_entityQuery.get< point_animation_data>();
            //animate
            m_scheduler->queueJobs(m_entityQuery.size(), [&]()
                {
                    auto value = async::this_job::get_id();
                    Animate(positions[value], animData[value], deltaTime);
                }).wait();
                //bulk write
                m_entityQuery.submit<position>();
                m_entityQuery.submit<point_animation_data>();
        }

    private:
        void Animate(position& pos, point_animation_data& data, float deltaTime)
        {
            OPTICK_EVENT();

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
                }
            }


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

        ecs::EntityQuery m_entityQuery = createQuery<point_animation_data, position>();
        ecs::EntityQuery m_CamQuery = createQuery<camera>();
        math::vec3 m_camPos;

        const float m_speed = 1.0f;
        const float m_threshold = 3.5f;
    };

}
