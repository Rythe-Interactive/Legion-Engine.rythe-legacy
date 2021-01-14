#pragma once
#include <core/math/glm/vec3.hpp>
#include <array>
namespace legion::physics
{
    struct triangle
    {
        std::array<math::vec3,3> vertices;
    };
}
