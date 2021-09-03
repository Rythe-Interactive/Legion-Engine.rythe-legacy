#include <core/serialization/serializationregistry.hpp>
#pragma once

namespace legion::core::serialization
{
    std::map<id_type, std::unique_ptr<serializer_base>> serializer_registry::serializers;

    template<typename type>
    inline pointer<serializer<type>> serializer_registry::register_serializer()
    {
        serializer_registry::serializers.emplace(type_hash<type>().local(), std::make_unique<serializer<type>>());
        return get_serializer<type>();
    }

    template<typename type>
    inline pointer<serializer<type>> serializer_registry::get_serializer()
    {
        id_type typeId = type_hash<type>().local();
        if (serializers.count(typeId))
            return { dynamic_cast<serializer<type>*>(serializers.at(typeId).get()) };
        return register_serializer<type>();
    }

    template<typename type>
    inline pointer<serializer<type>> legion::core::serialization::serializer_registry::get_serializer(id_type id)
    {
        if (serializers.count(id))
            return { dynamic_cast<serializer<type>*>(serializers.at(id).get()) };
        return { nullptr };
    }
}
