#pragma once

#include <expected>
#include <string_view>
#include <system_error>
#include <type_traits>

namespace nebula::http::detail::router_utils
{
enum class SegmentKind
{
    Literal,
    Parameter,
    CatchAll,
};

enum class ParseSegmentError
{
    EmptyName,
    InvalidSyntax,
};

class ParseSegmentCategory final : public std::error_category
{
  public:
    const char *name() const noexcept override;
    
    std::string message(int errorCode) const override;
};

const std::error_category& parse_segment_category();

std::error_code make_error_code(ParseSegmentError error) noexcept;

struct ParsedSegment
{
    SegmentKind kind = SegmentKind::Literal;
    std::string_view value;
};

constexpr bool is_valid_segment_symbol(char c) noexcept;

constexpr bool is_valid_segment(std::string_view segment) noexcept;

constexpr bool is_paramater_segment(std::string_view segment) noexcept;

constexpr bool is_catch_all_segment(std::string_view segment) noexcept;

std::expected<ParsedSegment, std::error_code> parse_segment(std::string_view segment);
} // namespace nebula::http::detail

template<>
struct std::is_error_code_enum<nebula::http::detail::router_utils::ParseSegmentError> 
    : std::true_type {};
