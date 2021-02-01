#pragma once
#include <core/core.hpp>
#include <rendering/data/vertexarray.hpp>
#include <rendering/data/buffer.hpp>
#include <core/compute/high_level/function.hpp>

namespace legion::rendering
{

    struct point_cloud_particle_container
    {
    public:
        bool buffered = false;
        vertexarray vertexArray;
        buffer positionBuffer;
        buffer colorBuffer;
        buffer emissionBuffer;
        buffer normalBuffer;

        std::vector<ecs::entity_handle> livingParticles;
        std::vector<ecs::entity_handle> deadParticles;
        std::vector<math::color> colorBufferData;
        std::vector<math::color> emissionBufferData;
        std::vector <math::vec3> positionBufferData;
        std::vector <math::vec3> normalBufferData;
        std::vector<byte> isAnimating;

        compute::function pointUpdateCL;

    };
}
