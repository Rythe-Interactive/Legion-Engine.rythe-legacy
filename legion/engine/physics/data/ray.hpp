#pragma once
#include <core/math/glm/vec3.hpp>

namespace legion::physics
{
    struct ray
    {
        core::math::vec3 origin;
        core::math::vec3 direction;
    };
}
