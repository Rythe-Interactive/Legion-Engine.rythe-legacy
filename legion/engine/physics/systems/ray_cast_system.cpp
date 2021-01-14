#include "ray_cast_system.hpp"

namespace legion::physics
{
    ecs::EntityQuery  RayCastSystem::slowQuery;
    ecs::EntityQuery  RayCastSystem::fastQuery;
    ecs::EntityQuery  RayCastSystem::physicsQuery;
    events::EventBus* RayCastSystem::eventBus;
}
