#pragma once

#include <expected>
#include <system_error>

namespace pulsar
{
template<typename T>
using Result = std::expected<T, std::error_code>;
}
