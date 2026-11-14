#pragma once

#include "nebula/http/request_context.hpp"
#include <functional>

namespace nebula::http
{
using Handler = std::move_only_function<void(RequestContext)>;
} // namespace nebula:http
