#include "nebula/http/detail/router_utils.hpp"
#include "nebula/http/detail/string_utils.hpp"

#include <algorithm>
#include <string_view>

namespace nebula::http::detail::router_utils
{
const char* ParseSegmentCategory::name() const noexcept
{
    return "nebula.http.router.segment";
}

std::string ParseSegmentCategory::message(int errorCode) const
{
    switch (static_cast<ParseSegmentError>(errorCode))
    {
        case ParseSegmentError::EmptyName:
            return "Empty parameter name";
        case ParseSegmentError::InvalidSyntax:
            return "Invalid segment syntax";
    }

    return "Unknown segment parse error";
}

const std::error_category& parse_segment_category()
{
    static ParseSegmentCategory category; 
    return category;
}

std::error_code make_error_code(ParseSegmentError error) noexcept
{
    return {static_cast<int>(error), parse_segment_category()};
}

constexpr bool is_valid_segment_symbol(char c) noexcept
{
    constexpr std::string_view allowed = "-._~!$&'()*+,;=:@";
    return string_utils::is_ascii_alnum(c) || std::ranges::contains(allowed, c); 
}

constexpr bool is_valid_segment(std::string_view segment) noexcept
{
    for (std::size_t i = 0; i < segment.size(); ++i)
    {
        if (segment[i] == '%')
        {
            if (segment.size() - i < 3
                || !string_utils::is_hex(segment[i + 1])
                || !string_utils::is_hex(segment[i + 2]))
            {
                return false;
            }

            i += 2;
        }
        else if (!is_valid_segment_symbol(segment[i]))
        {
            return false;
        }
    }

    return true;
}

std::expected<ParsedSegment, std::error_code> parse_segment(std::string_view segment)
{
    if (segment.empty())
    {
        return std::unexpected{make_error_code(ParseSegmentError::EmptyName)};
    }

    if (segment.size() >= 2 && segment.front() == '{' && segment.back() == '}')
    {
        std::string_view wildcard = segment.substr(1, segment.size() - 2);
        if (wildcard.empty())
        {
            return std::unexpected{make_error_code(ParseSegmentError::EmptyName)};
        }
        
        if (!std::ranges::all_of(wildcard, [](char c) {return isalpha(c);}))
        {
            return std::unexpected{make_error_code(ParseSegmentError::InvalidSyntax)};
        }        

        if (wildcard.front() == '+')
        {
            std::string_view catchAllParameter = wildcard.substr(1); 
            if (catchAllParameter.empty())
            {
                return std::unexpected{make_error_code(ParseSegmentError::EmptyName)};
            }

            return ParsedSegment{.kind = SegmentKind::CatchAll, .value = catchAllParameter};
        }

        return ParsedSegment{.kind = SegmentKind::Parameter, .value = wildcard};
    }

    if (!is_valid_segment(segment))
    {
        return std::unexpected{make_error_code(ParseSegmentError::InvalidSyntax)};
    }

    return ParsedSegment{.kind = SegmentKind::Literal, .value = segment};
}
} // namespace nebula::http::router_utils::detail
