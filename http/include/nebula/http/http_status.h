#pragma once

#include <cstdint>
#include <string_view>

namespace nebula::http
{
enum class HttpStatus : std::uint16_t
{
    // 1xx - информационные
    Continue = 100,
    SwitchingProtocols = 101,
    Processing = 102, // WebDAV, RFC 2518; устарел
    EarlyHints = 103,

    // 2xx - успех
    Ok = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultiStatus = 207,     // WebDAV, RFC 4918
    AlreadyReported = 208, // WebDAV, RFC 5842
    ImUsed = 226,          // RFC 3229

    // 3xx - перенаправление
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    UseProxy = 305, // устарел, RFC 9110 §15.4.6
    TemporaryRedirect = 307,
    PermanentRedirect = 308,

    // 4xx - ошибка клиента
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    ContentTooLarge = 413,
    UriTooLong = 414,
    UnsupportedMediaType = 415,
    RangeNotSatisfiable = 416,
    ExpectationFailed = 417,
    MisdirectedRequest = 421,
    UnprocessableContent = 422,
    Locked = 423,           // WebDAV, RFC 4918
    FailedDependency = 424, // WebDAV, RFC 4918
    TooEarly = 425,         // RFC 8470
    UpgradeRequired = 426,
    PreconditionRequired = 428,
    TooManyRequests = 429,
    RequestHeaderFieldsTooLarge = 431,
    UnavailableForLegalReasons = 451,

    // 5xx - ошибка сервера
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    VersionNotSupported = 505,
    VariantAlsoNegotiates = 506,
    InsufficientStorage = 507, // WebDAV, RFC 4918
    LoopDetected = 508,        // WebDAV, RFC 5842
    NotExtended = 510,
    NetworkAuthRequired = 511,
};

// Числовой код статуса.
[[nodiscard]] constexpr std::uint16_t toCode(HttpStatus status) noexcept
{
    return static_cast<std::uint16_t>(status);
}

// Первая цифра кода: 1..5 для валидных статусов, 0 для мусора.
[[nodiscard]] constexpr int statusClass(HttpStatus status) noexcept
{
    const auto code = toCode(status);
    return (code >= 100 && code <= 599) ? code / 100 : 0;
}

[[nodiscard]] constexpr bool isInformational(HttpStatus s) noexcept
{
    return statusClass(s) == 1;
}
[[nodiscard]] constexpr bool isSuccessful(HttpStatus s) noexcept
{
    return statusClass(s) == 2;
}
[[nodiscard]] constexpr bool isRedirection(HttpStatus s) noexcept
{
    return statusClass(s) == 3;
}
[[nodiscard]] constexpr bool isClientError(HttpStatus s) noexcept
{
    return statusClass(s) == 4;
}
[[nodiscard]] constexpr bool isServerError(HttpStatus s) noexcept
{
    return statusClass(s) == 5;
}

[[nodiscard]] constexpr bool isError(HttpStatus s) noexcept
{
    const auto cls = statusClass(s);
    return cls == 4 || cls == 5;
}

// У этих статусов тела быть не должно - RFC 9110 §6.4.1.
[[nodiscard]] constexpr bool allowsBody(HttpStatus s) noexcept
{
    return !(isInformational(s) || s == HttpStatus::NoContent || s == HttpStatus::NotModified);
}

// Reason phrase по RFC 9110. Для неизвестного кода - "Unknown".
//
// switch намеренно без default: тогда -Wswitch напомнит дописать фразу,
// если в enum добавят новый статус.
[[nodiscard]] constexpr std::string_view reasonPhrase(HttpStatus status) noexcept
{
    switch (status)
    {
    case HttpStatus::Continue:
        return "Continue";
    case HttpStatus::SwitchingProtocols:
        return "Switching Protocols";
    case HttpStatus::Processing:
        return "Processing";
    case HttpStatus::EarlyHints:
        return "Early Hints";

    case HttpStatus::Ok:
        return "OK";
    case HttpStatus::Created:
        return "Created";
    case HttpStatus::Accepted:
        return "Accepted";
    case HttpStatus::NonAuthoritativeInformation:
        return "Non-Authoritative Information";
    case HttpStatus::NoContent:
        return "No Content";
    case HttpStatus::ResetContent:
        return "Reset Content";
    case HttpStatus::PartialContent:
        return "Partial Content";
    case HttpStatus::MultiStatus:
        return "Multi-Status";
    case HttpStatus::AlreadyReported:
        return "Already Reported";
    case HttpStatus::ImUsed:
        return "IM Used";

    case HttpStatus::MultipleChoices:
        return "Multiple Choices";
    case HttpStatus::MovedPermanently:
        return "Moved Permanently";
    case HttpStatus::Found:
        return "Found";
    case HttpStatus::SeeOther:
        return "See Other";
    case HttpStatus::NotModified:
        return "Not Modified";
    case HttpStatus::UseProxy:
        return "Use Proxy";
    case HttpStatus::TemporaryRedirect:
        return "Temporary Redirect";
    case HttpStatus::PermanentRedirect:
        return "Permanent Redirect";

    case HttpStatus::BadRequest:
        return "Bad Request";
    case HttpStatus::Unauthorized:
        return "Unauthorized";
    case HttpStatus::PaymentRequired:
        return "Payment Required";
    case HttpStatus::Forbidden:
        return "Forbidden";
    case HttpStatus::NotFound:
        return "Not Found";
    case HttpStatus::MethodNotAllowed:
        return "Method Not Allowed";
    case HttpStatus::NotAcceptable:
        return "Not Acceptable";
    case HttpStatus::ProxyAuthRequired:
        return "Proxy Authentication Required";
    case HttpStatus::RequestTimeout:
        return "Request Timeout";
    case HttpStatus::Conflict:
        return "Conflict";
    case HttpStatus::Gone:
        return "Gone";
    case HttpStatus::LengthRequired:
        return "Length Required";
    case HttpStatus::PreconditionFailed:
        return "Precondition Failed";
    case HttpStatus::ContentTooLarge:
        return "Content Too Large";
    case HttpStatus::UriTooLong:
        return "URI Too Long";
    case HttpStatus::UnsupportedMediaType:
        return "Unsupported Media Type";
    case HttpStatus::RangeNotSatisfiable:
        return "Range Not Satisfiable";
    case HttpStatus::ExpectationFailed:
        return "Expectation Failed";
    case HttpStatus::MisdirectedRequest:
        return "Misdirected Request";
    case HttpStatus::UnprocessableContent:
        return "Unprocessable Content";
    case HttpStatus::Locked:
        return "Locked";
    case HttpStatus::FailedDependency:
        return "Failed Dependency";
    case HttpStatus::TooEarly:
        return "Too Early";
    case HttpStatus::UpgradeRequired:
        return "Upgrade Required";
    case HttpStatus::PreconditionRequired:
        return "Precondition Required";
    case HttpStatus::TooManyRequests:
        return "Too Many Requests";
    case HttpStatus::RequestHeaderFieldsTooLarge:
        return "Request Header Fields Too Large";
    case HttpStatus::UnavailableForLegalReasons:
        return "Unavailable For Legal Reasons";

    case HttpStatus::InternalServerError:
        return "Internal Server Error";
    case HttpStatus::NotImplemented:
        return "Not Implemented";
    case HttpStatus::BadGateway:
        return "Bad Gateway";
    case HttpStatus::ServiceUnavailable:
        return "Service Unavailable";
    case HttpStatus::GatewayTimeout:
        return "Gateway Timeout";
    case HttpStatus::VersionNotSupported:
        return "HTTP Version Not Supported";
    case HttpStatus::VariantAlsoNegotiates:
        return "Variant Also Negotiates";
    case HttpStatus::InsufficientStorage:
        return "Insufficient Storage";
    case HttpStatus::LoopDetected:
        return "Loop Detected";
    case HttpStatus::NotExtended:
        return "Not Extended";
    case HttpStatus::NetworkAuthRequired:
        return "Network Authentication Required";
    }
    return "Unknown";
}
} // namespace nebula::http