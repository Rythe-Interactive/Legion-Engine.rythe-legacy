#pragma once
#include <memory>
#include <type_traits>

#include <core/types/primitives.hpp>
#include <core/types/meta.hpp>
#include <core/types/type_util.hpp>
#include <core/platform/platform.hpp>
#include <core/containers/sparse_map.hpp>
#include <core/ecs/registry.hpp>
#include <core/events/eventbus.hpp>
#include <core/scheduling/scheduling.hpp>

#include <core/engine/system.hpp>

/**
 * @file module.hpp
 */

namespace legion::core
{
    /**@class Module
     * @brief interface for engine-modules, must be implemented
     * @ref legion::core::Engine::reportModule<T,...>()
     */
    class Module
    {
        friend class Engine;
    private:
        multicast_delegate<void()> m_setupFuncs;

        std::unordered_map<id_type, std::unique_ptr<SystemBase>> m_systems;

        void init()
        {
            m_setupFuncs.invoke();
        };

    protected:
        template<size_type charc>
        void createProcessChain(const char(&name)[charc])
        {
            schd::Scheduler::createProcessChain<charc>(name);
        }

        template<typename SystemType, typename... Args CNDOXY(inherits_from<SystemType, System<SystemType>> = 0)>
        void reportSystem(Args&&... args)
        {
            SystemType* system = static_cast<SystemType*>(m_systems.emplace(make_hash<SystemType>(), std::make_unique<SystemType>(std::forward<Args>(args)...)).first->second.get());
            if constexpr (has_setup_v<SystemType, void()>)
            {
                m_setupFuncs.insert_back<SystemType, &SystemType::setup>(system);
            }
            if constexpr (has_update_v<SystemType, void(time::span)>)
            {
                system->template createProcess<&SystemType::update>("Update");
            }
        }

        template<typename component_type, typename... Args>
        void registerComponentType(Args&&... args)
        {
            ecs::Registry::registerComponentType<component_type>(std::forward<Args>(args)...);
        }

    public:
        virtual void setup() LEGION_PURE;

        /**@brief determines the execution priority of this module
         * @ref LEGION_IMPURE_RETURN
         * @returns priority_type signed int8 higher is higher priority and get called first
         * @note default priority of the engine is 0.
         * @note call order for modules with the same priority is undefined.
         */
        virtual priority_type priority() LEGION_IMPURE_RETURN(default_priority);

        virtual ~Module() = default;
    };
}
