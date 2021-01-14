#pragma once
#include <core/core.hpp>

namespace legion::core::math
{
    inline vec3 screen_to_world(
        const vec2& screen_pos,
        const vec3& position,
        const vec3& forward,
        const vec3& up,
        const vec3& right,
        float fovx,
        float width,
        float height
    )
    {
        const float alphaMax = tan(deg2rad(fovx) /2.0f) ;//betaMax * width / height;
        const float betaMax  = alphaMax / (width/height);

        const float halfWidth = width /2.0f;
        const float halfHeight = height / 2.0f;

        const float kAlpha = (screen_pos.x - halfWidth) / halfWidth;
        const float kBeta  = (halfHeight - screen_pos.y) / halfHeight;

        const float alpha = alphaMax * kAlpha;
        const float beta = betaMax * kBeta;

        return position + (right * alpha) + (up * beta) + forward;
    }
}
