#pragma once
#include <core/core.hpp>
#include <rendering/data/vertexarray.hpp>
#include <rendering/data/buffer.hpp>

namespace legion::rendering
{

    struct point_cloud_particle_container
    {
    public:
        bool buffered = false;
        vertexarray vertexArray;
        buffer positionBuffer;
        buffer colorBuffer;

        std::vector<ecs::entity_handle> livingParticles;
        std::vector<ecs::entity_handle> deadParticles;
        std::vector<math::color> colorBufferData;
        std::vector <math::vec3 > positionBufferData;
    };
}
