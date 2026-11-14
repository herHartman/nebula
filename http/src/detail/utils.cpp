#include "nebula/http/detail/utils.hpp"
#include "nebula/http/query_args.hpp"
#include <charconv>
#include <string_view>

namespace nebula::http::detail
{
enum class DecodeMode
{
    // RFC 3986: '+' is a literal
    Path,
    // x-www-form-urlencoded: '+' means space
    Form
};

// http://example.com/my%20file.txt
// TODO(agartman) use std::expected + ParseError
bool percent_decode(std::string_view in, std::string &out)
{
    out.clear();
    out.reserve(in.size());
    for (std::size_t idx = 0; idx < in.size(); ++idx)
    {
        if (in[idx] != '%')
        {
            out += in[idx];
            continue;
        }

        if (idx + 2 >= in.size())
        {
            return false;
        }

        const char *begin = in.data() + idx + 1;
        const char *end = begin + 2;
        unsigned value = 0;
        auto [ptr, ec] = std::from_chars(begin, end, value, 16);
        if (ec != std::errc() || ptr != end)
        {
            return false;
        }
        out += static_cast<char>(value);
        idx += 2;
    }

    return true;
}

// a=1&b&c=
bool parse_query(std::string_view in, QueryArgs &out)
{
    while (!in.empty())
    {
        const auto amp = in.find("&");
        const auto pair = in.substr(0, amp);
        in = amp == std::string_view::npos ? std::string_view{} : in.substr(amp + 1);
        if (pair.empty())
        {
            continue;
        }

        const auto eq = pair.find("=");
        std::string key;
        if (!percent_decode(pair.substr(0, eq), key))
        {
            return false;
        }

        std::string value;
        if (eq != std::string_view::npos)
        {
            if (!percent_decode(pair.substr(eq + 1), value))
            {
                return false;
            }
        }
        out.insert_or_assign(std::move(key), std::move(value));
    }
    return true;
}
} // namespace nebula::http::detail
