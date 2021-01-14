#pragma once
#include <core/core.hpp>
#include <core/data/mesh.hpp>
#include <core/math/screen_to_world.hpp>
#include <physics/data/ray.hpp>
#include <physics/data/triangle.hpp>

namespace legion::physics
{
    namespace detail
    {
        using namespace math;

        struct ray_hit_info
        {
            bool hit;
            float t = 0.0f, u = 0.0f, v = 0.0f;
            vec3 position, normal;
        };

        inline ray_hit_info rayTriangleIntersectMT97(const triangle& tri, const ray& ray)
        {

            // find vectors for two edges sharing vert0
            const vec3 edge1 = tri.vertices[0] - tri.vertices[1];
            const vec3 edge2 = tri.vertices[2] - tri.vertices[0];

            // begin calculating determinant - also used to calculate U parameter
            const vec3 pvec = cross(ray.direction, edge2);

            // if determinant is near zero, ray lies in plane of triangle
            const float det = dot(edge1, pvec);

            // use backface culling
            if (det < std::numeric_limits<float>::epsilon())
            {
                return ray_hit_info{ false,0,0,0 ,vec3(0),vec3(0)};
            }

            const float inv_det = 1.0f / det;

            // calculate distance from vert0 to ray origin
            const vec3 tvec = ray.origin - tri.vertices[0];

            // calculate U parameter and test bounds
            const float u = dot(tvec, pvec) * inv_det;
            if( u < 0.0f || u > 1.0f)
            {
                return ray_hit_info{false,0.0,u,0.0f,vec3(0),vec3(0)};
            }

            // prepare to test V parameter
            vec3 qvec = cross(tvec,edge1);

            // calculate V parameter and test bounds
            const float v = dot(ray.direction,qvec) * inv_det;
            if( v < 0.0f || u + v > 1.0f)
            {
                return ray_hit_info{false,0.0,u,v,vec3(0),vec3(0)};
            }

            //calculate remaining t
            const float t = dot(edge2,qvec) * inv_det;

            return ray_hit_info{true,t,u,v,ray.origin + ray.direction * t,normalize(cross(edge1,edge2))};
        }

        inline ray_hit_info rayTriangleIntersectTransformed(const triangle& tri,const ray& r,const mat4& objectTransform)
        {
            ray transformed;
            const mat4 invObjTransform = inverse(objectTransform);

            //transform ray into triangles object space
            transformed.direction = normalize(invObjTransform * vec4(r.direction,0) );
            transformed.origin = invObjTransform * vec4(r.origin,1); 

            //do intersection
            auto info = rayTriangleIntersectMT97(tri,transformed);

            //transform result back into world space
            info.normal = normalize(transpose(inverse(objectTransform)) * vec4(info.normal,0)).xyz;
            info.position = (objectTransform * vec4(info.position,1)).xyz;

            return info;
        }

        inline ray_hit_info rayMeshIntersect(const ray& r,const mesh& m, const mat4& objectTransform)
        {
            ray_hit_info result{false,std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max()};

            //check all triangles in mesh
            for(size_type itr = 0; itr < m.indices.size(); itr +=3)
            {
                triangle tri;

                tri.vertices[0] = m.vertices[m.indices[itr+0]];
                tri.vertices[1] = m.vertices[m.indices[itr+1]];
                tri.vertices[2] = m.vertices[m.indices[itr+2]];

                //TODO(algo-ryth-mix): conceivably there is a speed up here ?
                // consider
                // v_a = avg(tri.vertices)
                // rough_position = r.origin - v_a
                // if rough_position > result.t + accuracy_bias then discard 


                //do triangle intersection
                ray_hit_info info = rayTriangleIntersectTransformed(tri,r,objectTransform);


                //check result
                if(info.hit)
                {
                    const float thisT = length(r.origin - info.position);
                    if(thisT < result.t)
                    {
                        result = info;
                        result.t = thisT;
                    }
                }
            }
            return result;
        }
    }
}
