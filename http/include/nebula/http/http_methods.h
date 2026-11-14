#pragma once

#include <cstdint>

namespace nebula::http
{
enum class HttpMethods : std::uint16_t
{
    Options = 1 << 0,
    Get = 1 << 1,
    Head = 1 << 2,
    Post = 1 << 3,
    Put = 1 << 4,
    Patch = 1 << 5,
    Delete = 1 << 6,
};
} // namespace nebula::http
