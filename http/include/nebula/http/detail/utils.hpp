#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>

namespace nebula::http::detail
{
bool percent_decode(std::string_view in, std::string &out);

bool parse_query(std::string_view in, std::unordered_map<std::string, std::string>& out);
} // namespace nebula::http::detail

