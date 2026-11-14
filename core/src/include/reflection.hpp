#pragma once

#include <algorithm>
#include <array>
#include <meta>
#include <string_view>
#include <type_traits>

namespace nebula::core::reflection
{
template <typename T>
concept Enumeration = std::is_enum_v<T>;

template <Enumeration T>
struct EnumItem
{
    std::string_view name;
    T value;
};

template <Enumeration E>
consteval auto inspect_enum()
{
    constexpr auto members = std::define_static_array(std::meta::enumerators_of(^^E));

    std::array<EnumItem<E>, members.size()> result;

    std::ranges::transform(members, result.begin(), [](std::meta::info m) {
        return EnumItem<E>{std::meta::identifier_of(m), std::meta::extract<E>(m)};
    });

    return result;
}
} // namespace nebula::core::reflection
