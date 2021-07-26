#include <core/serialization/view_type.hpp>
#pragma once

namespace legion::core::serialization
{
    template<typename type>
    inline json json_view<type>::serialize(type object)
    {
        json j;
        j["value"] = 10;
        return j;
    }

    template<typename type>
    inline prototype_base json_view<type>::deserialize(json j)
    {
        return prototype<type>();
    }
}

