#pragma once

#include "nebula/http/query_args.hpp"

#include <string>
#include <string_view>

namespace nebula::http::detail
{
bool percent_decode(std::string_view in, std::string &out);

bool parse_query(std::string_view in, QueryArgs &out);
} // namespace nebula::http::detail
