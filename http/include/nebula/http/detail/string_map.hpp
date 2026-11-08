#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace nebula::http::detail
{
struct StringHash
{
    using is_transparent = void;
    std::size_t operator()(std::string_view s) const noexcept
    {
        return std::hash<std::string_view>{}(s);
    }
};

template <typename T>
using StringMap = std::unordered_map<std::string, T, StringHash, std::equal_to<>>;
} // namespace nebula::http::detail
