#pragma once
#include <core/core.hpp>
#include <physics/components/physics_component.hpp>
#include <physics/components/ray_intersectable.hpp>
#include <physics/ray_utils/ray_intersect.hpp>
#include <physics/data/ray.hpp>
#include <physics/events/events.hpp>
#include <rendering/debugrendering.hpp>

namespace legion::physics {
    class RayCastSystem : public System<RayCastSystem>
    {
        static ecs::EntityQuery slowQuery;
        static ecs::EntityQuery fastQuery;
        static ecs::EntityQuery physicsQuery;
        static events::EventBus* eventBus;

        void setup() override
        {
            slowQuery =            createQuery<mesh_filter, transform>();
            fastQuery =            createQuery<ray_intersection_marker, mesh_filter, transform>();
            physicsQuery =         createQuery<physicsComponent, mesh_filter, transform>();
            eventBus = m_eventBus;
            
        }

    public:

        enum IntersectionMode
        {
            ALL,
            WITH_MARKER,
            PHYSIC_OBJECTS
        };

        static void dispatchRayCast(const ray& r, IntersectionMode mode = WITH_MARKER)
        {
            log::debug("y u no cast rey ?");
            debug::drawLine(r.origin,r.origin + r.direction * 100,math::colors::red,10,0,true);


            struct params { ray r; IntersectionMode mode; };

            auto* p = new params{ r,mode };

            m_scheduler->queueJob([](void* data)
            {
                auto& [r, mode] = *static_cast<params*>(data);

                ecs::EntityQuery& entities = slowQuery;

                switch (mode)
                {
                case ALL:
                    entities = slowQuery;
                    break;
                case WITH_MARKER:
                    entities = fastQuery;
                    break;
                case PHYSIC_OBJECTS:
                    entities = physicsQuery;
                    break;
                }

                detail::ray_hit_info result{false};
                ecs::entity_handle target;

                log::debug("Entities in Query of Raycast: {}",entities.size());

                int i = 0;

                entities.queryEntities();
                for(ecs::entity_handle entity : entities)
                {

                    log::debug("checking entity {} of {}",++i,entities.size());
                    auto mfilter = entity.read_component<mesh_filter>();
                    auto tr = entity.read_component<transform>();
                    auto [lock,mesh] = mfilter.get();
                    detail::ray_hit_info info;

                    lock.critical_section<async::readonly_guard>([&,r=r,mesh=mesh]
                    {
                        info = detail::rayMeshIntersect(r,mesh,tr.get_local_to_parent_matrix());
                    });

                    if(info.hit)
                    {
                        if(result.hit)
                        {
                            if(info.t < result.t)
                            {
                                result = info;
                                target = entity;
                            }
                        }
                        else
                        {
                            result = info;
                            target = entity;
                        }
                    }
                }

                if(result.hit)
                {
                    ray_collision_event rce;
                    rce.event_data = result;
                    rce.target = target;
                    eventBus->raiseEvent<ray_collision_event>(rce);
                    log::debug("I hit something!");
                }

                delete static_cast<params*>(data);

            },p);
        }
    };
}
