#pragma once

#include <core/core.hpp>
#include <memory>
#include <physics/halfedgeface.hpp>


namespace legion::physics
{

    //for debugging purposes only
    struct TempLine
    {
        math::vec3 start;
        math::vec3 end;
    };

    struct physicsComponent;
    struct physics_manifold;
    class ConvexCollider;


    class PhysicsCollider
    {
    public:

        PhysicsCollider() = default;

        /** @brief given a PhysicsCollider, CheckCollision calls "CheckCollisionWith". Both colliders are then passed through
        * to the correct "CheckCollisionWith" function with double dispatch.
        * @param physicsCollider The collider we would like to check collision against
        * @param [in/out] manifold A physics_manifold that holds information about the collision
        */
        virtual void CheckCollision(
            std::shared_ptr<PhysicsCollider> physicsCollider, physics_manifold& manifold) {};

        /** @brief given a convexCollider checks if this collider collides the convexCollider. The information
        * the information is then passed to the manifold.
        */
        virtual void CheckCollisionWith(ConvexCollider* convexCollider, physics_manifold& manifold) {};

        /** @brief given a PhysicsCollider, PopulateContactPoints calls PopulateContactPointsWith. Both colliders are then passed through
        * to the corrent FillManifoldWith function with double dispatch.
        */
        virtual void PopulateContactPoints(
            std::shared_ptr<PhysicsCollider> physicsCollider, physics_manifold& manifold) {};

        /** @brief Creates the contact points between this physics collider and the given ConvexCollider and
        * stores them in the manifold
        */
        virtual void PopulateContactPointsWith(
            ConvexCollider* convexCollider, physics_manifold& manifold) {};


        /** @brief Given the transform of the entity that the collider is attached to, draws a visual representation
        * of the collider.
        * @note This is called internally by PhysicsSysten
        */
        inline virtual void DrawColliderRepresentation(math::mat4 transform) {};

        inline virtual std::vector<HalfEdgeFace*>& GetHalfEdgeFaces()
        {
            return dummyHalfEdges;
        }

        // this is only here for debug reasons
        std::vector<TempLine> collisionsFound;

    private:
       //this is not used, its mostly for debug reasons
        std::vector<HalfEdgeFace*> dummyHalfEdges;

    };
}


