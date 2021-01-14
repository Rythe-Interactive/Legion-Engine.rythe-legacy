#pragma once
#include <core/core.hpp>
#include <core/math/screen_to_world.hpp>
#include <physics/data/ray.hpp>


namespace legion::physics
{
    inline ray rayFromScreenCoords(math::vec2 screenPos,transform camera_transform,float fovx,math::vec2 screenSize)
    {
        const position pos = camera_transform.get<position>().read();
        const rotation rot = camera_transform.get<rotation>().read();

        const math::vec3 direction = screen_to_world(screenPos,pos,rot.forward(),rot.up(),rot.right(),fovx,screenSize.x,screenSize.y);

        return ray{pos,normalize(direction-pos)};
    }
}
