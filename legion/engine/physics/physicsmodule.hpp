#pragma once

#include <core/core.hpp>
#include <physics/systems/physicssystem.hpp>
#include <physics/components/physics_component.hpp>
#include <physics/components/rigidbody.hpp>
#include <physics/mesh_splitter_utils/mesh_splitter.hpp>

#include "systems/ray_cast_system.hpp"

namespace legion::physics
{
    class PhysicsModule : public Module
    {
    public:

        void setup() override
        {
            addProcessChain("Physics");
            reportSystem<PhysicsSystem>();
            reportSystem<RayCastSystem>();
            reportComponentType<physicsComponent>();
            reportComponentType<rigidbody>();
            reportComponentType<identifier>();
            reportComponentType<MeshSplitter>();
            reportComponentType<ray_intersection_marker>();
            //reportComponentType <addRB>();
        }

        priority_type priority() override
        {
            return 20;
        }

    };

}


