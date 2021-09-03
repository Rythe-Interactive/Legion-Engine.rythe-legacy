#pragma once
#include <core/core.hpp>
#include <physics/broadphasecollisionalgorithms/broadphaseuniformgridnocaching.hpp>
#include <physics/broadphasecollisionalgorithms/broadphasecollisionalgorithm.hpp>
#include <physics/broadphasecollisionalgorithms/broadphaseuniformgrid.hpp>
#include <physics/broadphasecollisionalgorithms/broadphasebruteforce.hpp>
#include <physics/components/rigidbody.hpp>
#include <physics/data/physics_manifold_precursor.hpp>
#include <physics/data/physics_manifold.hpp>
#include <physics/physics_contact.hpp>
#include <physics/components/physics_component.hpp>
#include <physics/data/identifier.hpp>
#include <physics/events/events.hpp>
#include <memory>
#include <rendering/debugrendering.hpp>
#include <physics/components/fracturer.hpp>
#include <physics/components/fracturecountdown.hpp>

namespace legion::physics
{
    struct MeshLine
    {
        math::vec3 start;
        math::vec3 end;
        math::color Color;
    };

    class PhysicsSystem final : public System<PhysicsSystem>
    {
    public:
        static bool IsPaused;
        static bool oneTimeRunActive;

        ecs::filter<position, rotation, scale, physicsComponent> manifoldPrecursorQuery;

        //TODO move implementation to a seperate cpp file

        void setup()
        {
            createProcess<&PhysicsSystem::fixedUpdate>("Physics", m_timeStep);

            //std::make_unique<BroadphaseUniformGrid>(math::vec3(2,2,2),1);
            //std::make_unique<BroadphaseBruteforce>();
            //std::make_unique<BroadphaseUniformGridNoCaching>(math::vec3(2, 2, 2));

            m_broadPhase = std::make_unique<BroadphaseUniformGridNoCaching>(math::vec3(3, 3, 3));
        }

        void fixedUpdate(time::time_span<fast_time> deltaTime)
        {
            static time::timer physicsTimer;
            //log::debug("{}ms", physicsTimer.restart().milliseconds());
            OPTICK_EVENT();

            //static time::timer pt;
            //log::debug("frametime: {}ms", pt.restart().milliseconds());

            std::vector<ecs::component<rigidbody>> rigidbodies;
            std::vector<byte> hasRigidBodies;
            size_type size = manifoldPrecursorQuery.size();

            {
                OPTICK_EVENT("Fetching data");
                rigidbodies.resize(size);
                hasRigidBodies.resize(size);

                queueJobs(size, [&]() {
                    id_type index = async::this_job::get_id();
                    auto entity = manifoldPrecursorQuery[index];
                    if (entity.has_component<rigidbody>())
                    {
                        hasRigidBodies[index] = true;
                        rigidbodies[index] = entity.get_component<rigidbody>();
                    }
                    else
                        hasRigidBodies[index] = false;
                    }).wait();
            }

            auto& physComps = manifoldPrecursorQuery.get<physicsComponent>();
            auto& positions = manifoldPrecursorQuery.get<position>();
            auto& rotations = manifoldPrecursorQuery.get<rotation>();
            auto& scales = manifoldPrecursorQuery.get<scale>();

            if (!IsPaused)
            {
                integrateRigidbodies(hasRigidBodies, rigidbodies, deltaTime);
                runPhysicsPipeline(hasRigidBodies, rigidbodies, physComps, positions, rotations, scales, deltaTime);
                integrateRigidbodyQueryPositionAndRotation(hasRigidBodies, positions, rotations, rigidbodies, deltaTime);
            }

            if (oneTimeRunActive)
            {
                oneTimeRunActive = false;

                integrateRigidbodies(hasRigidBodies, rigidbodies, deltaTime);
                runPhysicsPipeline(hasRigidBodies, rigidbodies, physComps, positions, rotations, scales, deltaTime);
                integrateRigidbodyQueryPositionAndRotation(hasRigidBodies, positions, rotations, rigidbodies, deltaTime);
            }

            {
                OPTICK_EVENT("Writing data");
                queueJobs(size, [&]() {
                    id_type index = async::this_job::get_id();
                    if (hasRigidBodies[index])
                    {
                        auto entity = manifoldPrecursorQuery[index];
                        entity.get_component<rigidbody>() = rigidbodies[index];
                    }
                    }).wait();

                    //collisionPrecursorQuery.submit<physicsComponent>();
                    //collisionPrecursorQuery.submit<position>();
                    //collisionPrecursorQuery.submit<rotation>();
            }

            /* auto splitterDrawQuery = createQuery<MeshSplitter>();
             splitterDrawQuery.queryEntities();

             for (auto ent : splitterDrawQuery)
             {
                 auto splitterHandle = ent.get_component_handle<MeshSplitter>();

                 if (splitterHandle )
                 {
                     auto [posH,rotH,scaleH] = ent.get_component_handles<transform>();
                     debug::drawLine
                     (posH.read(), posH.read() + math::vec3(0, 5, 0), math::colors::red, 20.0f, 0.0f, true);

                     auto splitter = splitterHandle.read();
                     math::mat4 transform = math::compose(scaleH.read(), rotH.read(), posH.read());

                     splitter.DEBUG_DrawPolygonData(transform);
                 }
             }*/


        }

        void bulkRetrievePreManifoldData(
            ecs::component_container<physicsComponent>& physComps,
            ecs::component_container<position>& positions,
            ecs::component_container<rotation>& rotations,
            ecs::component_container<scale>& scales,
            std::vector<physics_manifold_precursor>& collisionPrecursors)
        {
            OPTICK_EVENT();
            collisionPrecursors.resize(physComps.size());

            queueJobs(physComps.size(), [&]() {
                id_type index = async::this_job::get_id();
                math::mat4 transf;
                math::compose(transf, scales[index].get(), rotations[index].get(), positions[index].get());

                for (auto& collider : physComps[index].get().colliders)
                    collider->UpdateTransformedTightBoundingVolume(transf);

                collisionPrecursors[index] = { transf, &physComps[index].get(), index, manifoldPrecursorQuery[index] };
                }).wait();
        }

        /**@brief Sets the broad phase collision detection method
         * Use BroadPhaseBruteForce to not use any broad phase collision detection
         */
        template <typename BroadPhaseType, typename ...Args>
        static void setBroadPhaseCollisionDetection(Args&& ...args)
        {
            static_assert(std::is_base_of_v<BroadPhaseCollisionAlgorithm, BroadPhaseType>, "Broadphase type did not inherit from BroadPhaseCollisionAlgorithm");
            m_broadPhase = std::make_unique<BroadPhaseType>(std::forward<Args>(args)...);
        }

        static void drawBroadPhase()
        {
            m_broadPhase->debugDraw();
        }

    private:

        static std::unique_ptr<BroadPhaseCollisionAlgorithm> m_broadPhase;
        const float m_timeStep = 0.02f;


        math::ivec3 uniformGridCellSize = math::ivec3(1, 1, 1);

        /** @brief Performs the entire physics pipeline (
         * Broadphase Collision Detection, Narrowphase Collision Detection, and the Collision Resolution)
        */
        void runPhysicsPipeline(
            std::vector<byte>& hasRigidBodies,
            std::vector<ecs::component<rigidbody>>& rigidbodies,
            std::vector<ecs::component<physicsComponent>>& physComps,
            std::vector<ecs::component<position>>& positions,
            std::vector<ecs::component<rotation>>& rotations,
            std::vector<ecs::component<scale>>& scales,
            float deltaTime)
        {
            OPTICK_EVENT();

            //-------------------------------------------------Broadphase Optimization-----------------------------------------------//

            //get all physics components from the world
            std::vector<physics_manifold_precursor> manifoldPrecursors;
            bulkRetrievePreManifoldData(physComps, positions, rotations, scales, manifoldPrecursors);

            std::vector<std::vector<physics_manifold_precursor>> manifoldPrecursorGrouping;
            //m_optimizeBroadPhase(manifoldPrecursors, manifoldPrecursorGrouping);
            manifoldPrecursorGrouping = m_broadPhase->collectPairs(std::move(manifoldPrecursors));

            //------------------------------------------------------ Narrowphase -----------------------------------------------------//
            std::vector<physics_manifold> manifoldsToSolve;

            {
                OPTICK_EVENT("Narrowphase");

                std::set<std::pair<id_type, id_type>> idPairings;

                size_type totalChecks = 0;
                for (auto& manifoldPrecursor : manifoldPrecursorGrouping)
                {
                    if (manifoldPrecursor.size() == 0) continue;
                    for (int i = 0; i < manifoldPrecursor.size() - 1; i++)
                    {
                        for (int j = i + 1; j < manifoldPrecursor.size(); j++)
                        {
                            totalChecks++;
                            assert(j != manifoldPrecursor.size());

                            physics_manifold_precursor& precursorA = manifoldPrecursor.at(i);
                            physics_manifold_precursor& precursorB = manifoldPrecursor.at(j);

                            //check if we have found this pairing before
                            std::pair<id_type, id_type> precursorPairing = std::make_pair(precursorA.id, precursorB.id);
                            auto foundPairingIter = idPairings.find(precursorPairing);

                            if (foundPairingIter != idPairings.end())
                            {
                                //early out so we dont solve the collision twice
                                continue;
                            }
                            else
                            {
                                idPairings.insert(precursorPairing);
                            }


                            auto& precursorPhyCompA = *precursorA.physicsComp;
                            auto& precursorPhyCompB = *precursorB.physicsComp;

                            auto& precursorRigidbodyA = rigidbodies[precursorA.id];
                            auto& precursorRigidbodyB = rigidbodies[precursorB.id];

                            //only construct a manifold if at least one of these requirement are fulfilled
                            //1. One of the physicsComponents is a trigger and the other one is not
                            //2. One of the physicsComponent's entity has a rigidbody and the other one is not a trigger
                            //3. Both have a rigidbody

                            bool isBetweenTriggerAndNonTrigger =
                                (precursorPhyCompA.isTrigger && !precursorPhyCompB.isTrigger) || (!precursorPhyCompA.isTrigger && precursorPhyCompB.isTrigger);

                            bool isBetweenRigidbodyAndNonTrigger =
                                (hasRigidBodies[precursorA.id] && !precursorPhyCompB.isTrigger) || (hasRigidBodies[precursorB.id] && !precursorPhyCompA.isTrigger);

                            bool isBetween2Rigidbodies = (hasRigidBodies[precursorA.id] && hasRigidBodies[precursorB.id]);


                            if (isBetweenTriggerAndNonTrigger || isBetweenRigidbodyAndNonTrigger || isBetween2Rigidbodies)
                            {
                                constructManifoldsWithPrecursors(rigidbodies, hasRigidBodies, precursorA, precursorB,
                                    manifoldsToSolve,
                                    hasRigidBodies[precursorA.id] || hasRigidBodies[precursorB.id]
                                    , precursorPhyCompA.isTrigger || precursorPhyCompB.isTrigger);
                            }
                        }
                    }
                }
                //log::debug("groupings {}", manifoldPrecursorGrouping.size());
                //log::debug("total checks {}", totalChecks);
            }

            //------------------------------------------------ Pre Collision Solve Events --------------------------------------------//


                //explosion event
            ecs::filter<FractureCountdown> countdownQuery;

            for (auto ent : countdownQuery)
            {
                auto fractureCountdown = ent.get_component<FractureCountdown>().get();
                fractureCountdown.fractureTime -= 0.02f;
                //log::debug(" fractureCountdown.fractureTime {}", fractureCountdown.fractureTime);

                if (fractureCountdown.explodeNow || fractureCountdown.fractureTime < 0.0f)
                {
                    log::debug("Entity is exploding");

                    auto fracturer = ent.get_component<Fracturer>().get();
                    log::debug("fractureCountdown.explosionPoint {} ", fractureCountdown.explosionPoint);
                    log::debug("fractureCountdown.fractureStrength {} ", fractureCountdown.fractureStrength);
                    FractureParams params(fractureCountdown.explosionPoint, fractureCountdown.fractureStrength);

                    fracturer.ExplodeEntity(ent, params);
                }
            }


            // all manifolds are initially valid

            std::vector<byte> manifoldValidity(manifoldsToSolve.size(), true);

            ////TODO we are currently hard coding fracture, this should be an event at some point
            //{
            //    OPTICK_EVENT("Fracture");
            //    for (size_t i = 0; i < manifoldsToSolve.size(); i++)
            //    {
            //        auto& manifold = manifoldsToSolve.at(i);

            //        auto& entityHandleA = manifold.entityA;
            //        auto& entityHandleB = manifold.entityB;

            //        auto fracturerHandleA = entityHandleA.get_component_handle<Fracturer>();
            //        auto fracturerHandleB = entityHandleB.get_component_handle<Fracturer>();

            //        bool currentManifoldValidity = manifoldValidity.at(i);

            //        //log::debug("- A Fracture Check");

            //        if (fracturerHandleA)
            //        {
            //            auto fracturerA = fracturerHandleA.read();
            //            //log::debug(" A is fracturable");
            //            fracturerA.HandleFracture(manifold, currentManifoldValidity, true);

            //            fracturerHandleA.write(fracturerA);
            //        }

            //        //log::debug("- B Fracture Check");

            //        if (fracturerHandleB)
            //        {
            //            auto fracturerB = fracturerHandleB.read();
            //            //log::debug(" B is fracturable");
            //            fracturerB.HandleFracture(manifold, currentManifoldValidity, false);

            //            fracturerHandleB.write(fracturerB);
            //        }

            //        manifoldValidity.at(i) = currentManifoldValidity;
            //    }
            //}
            //-------------------------------------------------- Collision Solver ---------------------------------------------------//
            //for both contact and friction resolution, an iterative algorithm is used.
            //Everytime physics_contact::resolveContactConstraint is called, the rigidbodies in question get closer to the actual
            //"correct" linear and angular velocity (Projected Gauss Seidel). For the sake of simplicity, an arbitrary number is set for the
            //iteration count.

            //the effective mass remains the same for every iteration of the solver. This means that we can precalculate it before
            //we start the solver

            //log::debug("--------------Logging contacts for manifold -------------------");
            {
                OPTICK_EVENT("Resolve collisions");

                initializeManifolds(manifoldsToSolve, manifoldValidity);

                auto largestPenetration = [](const physics_contact& contact1, const physics_contact& contact2)
                    -> bool {
                    return math::dot(contact1.RefWorldContact - contact1.IncWorldContact, -contact1.collisionNormal)
                > math::dot(contact2.RefWorldContact - contact2.IncWorldContact, -contact2.collisionNormal); };

                for (auto& manifold : manifoldsToSolve)
                {
                    std::sort(manifold.contacts.begin(), manifold.contacts.end(), largestPenetration);
                }

                {
                    OPTICK_EVENT("Resolve contact constraints");

                    //resolve contact constraint
                    for (size_t contactIter = 0;
                        contactIter < constants::contactSolverIterationCount; contactIter++)
                    {
                        resolveContactConstraint(manifoldsToSolve, manifoldValidity, deltaTime, contactIter);
                    }
                }

                {
                    OPTICK_EVENT("Resolve friction constraints");

                    //resolve friction constraint
                    for (size_t frictionIter = 0;
                        frictionIter < constants::frictionSolverIterationCount; frictionIter++)
                    {
                        resolveFrictionConstraint(manifoldsToSolve, manifoldValidity);
                    }
                }

                {
                    OPTICK_EVENT("Converge manifolds");

                    //reset convergance identifiers for all colliders
                    for (auto& manifold : manifoldsToSolve)
                    {
                        manifold.colliderA->converganceIdentifiers.clear();
                        manifold.colliderB->converganceIdentifiers.clear();
                    }

                    //using the known lambdas of this time step, add it as a convergance identifier
                    for (auto& manifold : manifoldsToSolve)
                    {
                        for (auto& contact : manifold.contacts)
                        {
                            contact.refCollider->AddConverganceIdentifier(contact);
                        }
                    }
                }
            }

        }
       
        /**@brief given 2 physics_manifold_precursors precursorA and precursorB, create a manifold for each collider in precursorA
        * with every other collider in precursorB. The manifolds that involve rigidbodies are then pushed into the given manifold list
        * @param manifoldsToSolve [out] a std::vector of physics_manifold that will store the manifolds created
        * @param isRigidbodyInvolved A bool that indicates whether a rigidbody is involved in this manifold
        * @param isTriggerInvolved A bool that indicates whether a physicsComponent with a physicsComponent::isTrigger set to true is involved in this manifold
        */
        void constructManifoldsWithPrecursors(ecs::component_container<rigidbody>& rigidbodies, std::vector<byte>& hasRigidBodies, physics_manifold_precursor& precursorA, physics_manifold_precursor& precursorB,
            std::vector<physics_manifold>& manifoldsToSolve, bool isRigidbodyInvolved, bool isTriggerInvolved)
        {
            OPTICK_EVENT();
            if (!precursorA.physicsComp || !precursorB.physicsComp) return;
            auto& physicsComponentA = *precursorA.physicsComp;
            auto& physicsComponentB = *precursorB.physicsComp;

            //if (physicsComponentA.colliders.empty() || physicsComponentB.colliders.empty()) return;

            for (auto colliderA : physicsComponentA.colliders)
            {
                for (auto colliderB : physicsComponentB.colliders)
                {
                    physics::physics_manifold m;
                    constructManifoldWithCollider(rigidbodies, hasRigidBodies, colliderA.get(), colliderB.get(), precursorA, precursorB, m);

                    if (!m.isColliding)
                    {
                        continue;
                    }

                    colliderA->PopulateContactPoints(colliderB.get(), m);

                    if (isTriggerInvolved)
                    {
                        //notify the event-bus
                        raiseEvent<trigger_event>(&m, m_timeStep);
                        //notify both the trigger and triggerer
                        //TODO:(Developer-The-Great): the triggerer and trigger should probably received this event
                        //TODO:(cont.) through the event bus, we should probably create a filterable system here to
                        //TODO:(cont.) uniquely identify involved objects and then redirect only required messages
                    }

                    if (isRigidbodyInvolved && !isTriggerInvolved)
                    {
                        raiseEvent<collision_event>(&m, m_timeStep);
                        manifoldsToSolve.emplace_back(std::move(m));
                    }
                }
            }
        }

        void constructManifoldWithCollider(
            ecs::component_container<rigidbody>& rigidbodies, std::vector<byte>& hasRigidBodies,
            PhysicsCollider* colliderA, PhysicsCollider* colliderB
            , physics_manifold_precursor& precursorA, physics_manifold_precursor& precursorB, physics_manifold& manifold)
        {
            OPTICK_EVENT();
            manifold.colliderA = colliderA;
            manifold.colliderB = colliderB;

            manifold.entityA = precursorA.entity;
            manifold.entityB = precursorB.entity;

            if (hasRigidBodies[precursorA.id])
                manifold.rigidbodyA = &rigidbodies[precursorA.id].get();
            else
                manifold.rigidbodyA = nullptr;

            if (hasRigidBodies[precursorB.id])
                manifold.rigidbodyB = &rigidbodies[precursorB.id].get();
            else
                manifold.rigidbodyB = nullptr;

            manifold.physicsCompA = precursorA.physicsComp;
            manifold.physicsCompB = precursorB.physicsComp;

            manifold.transformA = precursorA.worldTransform;
            manifold.transformB = precursorB.worldTransform;

            //manifold.DEBUG_checkID("floor", "problem");

            // log::debug("colliderA->CheckCollision(colliderB, manifold)");
            colliderA->CheckCollision(colliderB, manifold);
        }

        /** @brief gets all the entities with a rigidbody component and calls the integrate function on them
        */
        void integrateRigidbodies(std::vector<byte>& hasRigidBodies, ecs::component_container<rigidbody>& rigidbodies, float deltaTime)
        {
            OPTICK_EVENT();
           queueJobs(manifoldPrecursorQuery.size(), [&]() {
                if (!hasRigidBodies[async::this_job::get_id()])
                    return;

                auto& rb = rigidbodies[async::this_job::get_id()].get();

                ////-------------------- update velocity ------------------//
                math::vec3 acc = rb.forceAccumulator * rb.inverseMass;
                rb.velocity += (acc + constants::gravity) * deltaTime;

                ////-------------------- update angular velocity ------------------//
                math::vec3 angularAcc = rb.torqueAccumulator * rb.globalInverseInertiaTensor;
                rb.angularVelocity += (angularAcc)*deltaTime;

                rb.resetAccumulators();
                }).wait();
        }

        void integrateRigidbodyQueryPositionAndRotation(
            std::vector<byte>& hasRigidBodies,
            ecs::component_container<position>& positions,
            ecs::component_container<rotation>& rotations,
            ecs::component_container<rigidbody>& rigidbodies,
            float deltaTime)
        {
            OPTICK_EVENT();
            queueJobs(manifoldPrecursorQuery.size(), [&]() {
                id_type index = async::this_job::get_id();
                if (!hasRigidBodies[index])
                    return;

                auto& rb = rigidbodies[index].get();
                auto& pos = positions[index].get();
                auto& rot = rotations[index].get();

                ////-------------------- update position ------------------//
                pos += rb.velocity * deltaTime;

                ////-------------------- update rotation ------------------//
                float angle = math::clamp(math::length(rb.angularVelocity), 0.0f, 32.0f);
                float dtAngle = angle * deltaTime;

                if (!math::epsilonEqual(dtAngle, 0.0f, math::epsilon<float>()))
                {
                    math::vec3 axis = math::normalize(rb.angularVelocity);

                    math::quat glmQuat = math::angleAxis(dtAngle, axis);
                    rot = glmQuat * rot;
                    rot = math::normalize(rot);
                }

                //for now assume that there is no offset from bodyP
                rb.globalCentreOfMass = pos;

                rb.UpdateInertiaTensor(rot);
                }).wait();
        }

        void initializeManifolds(std::vector<physics_manifold>& manifoldsToSolve, std::vector<byte>& manifoldValidity)
        {
            OPTICK_EVENT();
            for (int i = 0; i < manifoldsToSolve.size(); i++)
            {
                if (manifoldValidity.at(i))
                {
                    auto& manifold = manifoldsToSolve.at(i);

                    for (auto& contact : manifold.contacts)
                    {
                        contact.preCalculateEffectiveMass();
                        contact.ApplyWarmStarting();
                    }
                }
            }
        }

        void resolveContactConstraint(std::vector<physics_manifold>& manifoldsToSolve, std::vector<byte>& manifoldValidity, float dt, int contactIter)
        {
            OPTICK_EVENT();

            for (int manifoldIter = 0;
                manifoldIter < manifoldsToSolve.size(); manifoldIter++)
            {
                if (manifoldValidity.at(manifoldIter))
                {
                    auto& manifold = manifoldsToSolve.at(manifoldIter);

                    for (auto& contact : manifold.contacts)
                    {
                        contact.resolveContactConstraint(dt, contactIter);
                    }
                }
            }
        }

        void resolveFrictionConstraint(std::vector<physics_manifold>& manifoldsToSolve, std::vector<byte>& manifoldValidity)
        {
            OPTICK_EVENT();

            for (int manifoldIter = 0;
                manifoldIter < manifoldsToSolve.size(); manifoldIter++)
            {
                if (manifoldValidity.at(manifoldIter))
                {
                    auto& manifold = manifoldsToSolve.at(manifoldIter);

                    for (auto& contact : manifold.contacts)
                    {
                        contact.resolveFrictionConstraint();
                    }
                }
            }
        }

    };
}

