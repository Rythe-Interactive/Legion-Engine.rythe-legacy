#pragma once
namespace legion::core::serialization
{
    struct prototype_base
    {
        virtual ~prototype_base() = default;
    };

    template<typename SrcType>
    struct prototype : public virtual prototype_base, public decltype(make_reflector(std::declval<SrcType>()))
    {
        using Reflector = decltype(make_reflector(std::declval<SrcType>()));

        prototype() : Reflector(make_reflector(SrcType())) {}
        prototype(const SrcType& src) : Reflector(make_reflector(src)) {}
        prototype(SrcType&& src) :Reflector(make_reflector(src)) {}
    };
}
