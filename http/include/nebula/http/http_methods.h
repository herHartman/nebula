#pragma once

#include <cstdint>

namespace nebula::http
{
enum class HttpMethods : std::uint16_t
{
    Options = 0,
    Get,
    Head,
    Post,
    Put,
    Patch,
    Delete,
    Trace,
    Connect,
    Count,
};
} // namespace nebula::http
