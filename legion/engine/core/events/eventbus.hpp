#pragma once
#include <memory>

#include <Optick/optick.h>

#include <core/containers/delegate.hpp>
#include <core/containers/sparse_map.hpp>
#include <core/containers/hashed_sparse_set.hpp>
#include <core/types/types.hpp>
#include <core/events/event.hpp>

/**@file eventbus.hpp
 */

namespace legion::core::events
{
    /**@class EventBus
     * @brief Central communication channel for events and messages.
     */
    class EventBus
    {
    private:
        static sparse_map<id_type, multicast_delegate<void(event_base&)>> m_eventCallbacks;

    public:
        /**@brief Insert event into bus and notify all subscribers.
         * @tparam event_type Event type to raise.
         * @param arguments Arguments to pass to the constructor of the event.
         */
        template<typename event_type, typename... Args, typename = inherits_from<event_type, event<event_type>>>
        static void raiseEvent(Args&&... arguments)
        {
            if (m_eventCallbacks.contains(event_type::id))
            {
                event_type event(arguments...); // Create new event.
                force_value_cast<multicast_delegate<void(event_type&)>>(m_eventCallbacks[event_type::id]).invoke(event); // Notify.
            }
        }

        /**@brief Non-templated raise event function. Inserts event into bus and notifies all subscribers.
         * @param value Reference to the event to insert into the bus.
         */
        static void raiseEvent(event_base& value)
        {
            if (m_eventCallbacks.contains(value.get_id()))
                m_eventCallbacks[value.get_id()].invoke(value);
        }

        /**@brief Unsafe, non-templated raise event function. This version is unsafe because it is allowed to trigger undefined behavior if the id is incompatible with the passed value.
         * @param value Reference to the event to insert into the bus.
         * @param id Type id of the event to invoke for. Overrides the polymorphic id of the reference passed as value.
         */
        static void raiseEventUnsafe(event_base& value, id_type id)
        {
            if (m_eventCallbacks.contains(id))
                m_eventCallbacks[id].invoke(value);
        }

        /**@brief Link a callback to an event type in order to get notified whenever one gets raised.
         * @tparam event_type Event type to subscribe to.
         */
        template<typename event_type, typename = inherits_from<event_type, event<event_type>>>
        static void bindToEvent(const delegate<void(event_type&)>& callback)
        {
            m_eventCallbacks[event_type::id].push_back(reinterpret_cast<const delegate<void(event_base&)>&>(callback));
        }

        /**@brief Non-templated function to link a callback to an event type in order to get notified whenever one gets raised.
         * @param id Type id of the event to subscribe to.
         * @param callback Function to bind.
         */
        static void bindToEvent(id_type id, const delegate<void(event_base&)>& callback)
        {
            m_eventCallbacks[id].push_back(callback);
        }

        /**@brief Link a callback to an event type in order to get notified whenever one gets raised.
         * @tparam event_type Event type to subscribe to.
         */
        template<typename event_type, typename = inherits_from<event_type, event<event_type>>>
        static void bindToEvent(delegate<void(event_type&)>&& callback)
        {
            m_eventCallbacks[event_type::id].push_back(reinterpret_cast<delegate<void(event_base&)>&&>(callback));
        }

        /**@brief Non-templated function to link a callback to an event type in order to get notified whenever one gets raised.
         * @param id Type id of the event to subscribe to.
         * @param callback Function to bind.
         */
        static void bindToEvent(id_type id, delegate<void(event_base&)>&& callback)
        {
            m_eventCallbacks[id].push_back(std::move(callback));
        }

        /**@brief Link a callback to an event type in order to get notified whenever one gets raised.
         * @tparam event_type Event type to subscribe to.
         */
        template<typename event_type, typename = inherits_from<event_type, event<event_type>>>
        static void unbindFromEvent(const delegate<void(event_type&)>& callback)
        {
            m_eventCallbacks[event_type::id].erase(reinterpret_cast<const delegate<void(event_base&)>&>(callback));
        }

        /**@brief Non-templated function to link a callback to an event type in order to get notified whenever one gets raised.
         * @param id Type id of the event to subscribe to.
         * @param callback Function to bind.
         */
        static void unbindFromEvent(id_type id, const delegate<void(event_base&)>& callback)
        {
            m_eventCallbacks[id].erase(callback);
        }
    };
}
