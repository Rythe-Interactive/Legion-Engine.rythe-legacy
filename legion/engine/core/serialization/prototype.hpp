#pragma once
namespace legion::core::serialization
{
    struct prototype_base
    {
        virtual ~prototype_base() = default;
    };

    template<typename SrcType>
    struct prototype : public prototype_base
    {

    };
}
