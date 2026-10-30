#include "nebula/http/request_parser.hpp"
#include "http_parser.h"
#include "nebula/http/detail/utils.hpp"
#include "nebula/http/http_methods.h"
#include "nebula/http/query_args.hpp"
#include <optional>
#include <string>
#include <utility>

namespace nebula::http
{
namespace
{
std::optional<HttpMethods> to_method(unsigned method)
{
    switch (static_cast<http_method>(method))
    {
    case HTTP_OPTIONS:
        return HttpMethods::Options;
    case HTTP_GET:
        return HttpMethods::Get;
    case HTTP_HEAD:
        return HttpMethods::Head;
    case HTTP_POST:
        return HttpMethods::Post;
    case HTTP_PUT:
        return HttpMethods::Put;
    case HTTP_PATCH:
        return HttpMethods::Patch;
    case HTTP_DELETE:
        return HttpMethods::Delete;
    case HTTP_TRACE:
        return HttpMethods::Trace;
    default:
        return std::nullopt;
    }
}
constexpr std::uint64_t kNoContentLength = std::numeric_limits<std::uint64_t>::max();
} // namespace

const http_parser_settings RequestParser::mSettings = []() {
    http_parser_settings settings{};
    settings.on_message_begin = RequestParser::onMessageBegin;
    settings.on_url = RequestParser::onUrl;
    settings.on_status = nullptr;
    settings.on_header_value = RequestParser::onHeaderValue;
    settings.on_header_field = RequestParser::onHeaderField;
    settings.on_headers_complete = RequestParser::onHeadersComplete;
    settings.on_body = nullptr;
    settings.on_message_complete = RequestParser::onMessageComplete;
    settings.on_chunk_header = nullptr;
    settings.on_chunk_complete = nullptr;
    return settings;
}();

RequestParser::RequestParser(ParserLimits limits) : mLimits(limits)
{
    http_parser_init(&mParser, HTTP_REQUEST);
    http_parser_url_init(&mUrlParser);
    mParser.data = this;
}

bool RequestParser::flushUrl()
{
    if (http_parser_parse_url(mUrl.data(), mUrl.size(), false, &mUrlParser) != 0)
    {
        mError = ParseError::BadRequest;
        return false;
    }
    
    const auto field = [&](http_parser_url_fields name) -> std::optional<std::string_view> {
        if ((mUrlParser.field_set & (1U << name)) == 0)
        {
            return std::nullopt;
        }
        return std::string_view{mUrl}.substr(mUrlParser.field_data[name].off, mUrlParser.field_data[name].len);
    };

    mRequestBuilder.appendPath(field(UF_PATH).value_or("/"));
    
    if (auto query = field(UF_QUERY))
    {
        QueryArgs queryArgs;
        if (!detail::parse_query(query.value(), queryArgs))
        {
            mError = ParseError::BadRequest;
            return false;
        }
        mRequestBuilder.appendQuery(std::move(queryArgs));
    }
    return true;
}

void RequestParser::flushHeader()
{
    mHeaderSize += mHeaderSize;
 
    auto end = mHeaderValue.find_first_not_of(" \t");
    if (end != std::string::npos)
    {
        mHeaderValue.erase(end + 1);
    }

    mRequestBuilder.appendHeader(mHeaderKey, mHeaderValue);
    mHeaderValue.clear();
    mHeaderKey.clear();
    mIsHeaderKey = true;
    mHeaderSize = 0;
}

int RequestParser::onMessageBegin(http_parser *parser)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onMessageBeginImpl(parser);
}

int RequestParser::onMessageComplete(http_parser *parser)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onMessageCompleteImpl(parser);
}

int RequestParser::onMessageBody(http_parser *parser, const char *data, std::size_t lenght)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onMessageBodyImpl(parser, data, lenght);
}

int RequestParser::onUrl(http_parser *parser, const char *data, std::size_t lenght)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onUrlImpl(parser, data, lenght);
}

int RequestParser::onStatus(http_parser *parser, const char *data, std::size_t lenght)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onStatusImpl(parser, data, lenght);
}

int RequestParser::onHeaderField(http_parser *parser, const char *data, std::size_t lenght)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onHeaderFieldImpl(parser, data, lenght);
}

int RequestParser::onHeaderValue(http_parser *parser, const char *data, std::size_t lenght)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onHeaderValueImpl(parser, data, lenght);
}

int RequestParser::onHeadersComplete(http_parser *parser)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onHeadersCompleteImpl(parser);
}

int RequestParser::onMessageBeginImpl(http_parser*)
{
    mState = ParseState::InProgress;
    return 0;
}

int RequestParser::onMessageCompleteImpl(http_parser *parser)
{
    mState = ParseState::Complete;
    http_parser_pause(parser, 1);
    return 0;
}

int RequestParser::onUrlImpl(http_parser*, const char *data, std::size_t len)
{
    if (mUrl.size() + len > kMaxUrlSize)
    {
       mError = ParseError::UriTooLong;
       return -1;
    }

    mUrl.append(data, data + len);
    return 0;
}

int RequestParser::onHeaderFieldImpl(http_parser*, const char *data, std::size_t len)
{
    if (!std::exchange(mIsHeaderKey, true))
    {
        flushHeader();
    }

    mHeaderSize += len;
    if (len > kMaxHeaderSize)
    {
        mError = ParseError::HeadersTooLarge;
        return -1;
    }

    mHeaderKey.append(data, len);
    return 0;
}

int RequestParser::onHeaderValueImpl(http_parser*, const char *data, std::size_t len)
{
    mIsHeaderKey = false;
    mHeaderSize += len;
    if (mHeaderSize > kMaxHeaderSize)
    {
        mError = ParseError::HeadersTooLarge;
        return -1;
    }

    mHeaderValue.append(data, len);
    return 0;
}

int RequestParser::onChunkHeader(http_parser *parser)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onChunkHeaderImpl(parser);
}

int RequestParser::onChunkComplete(http_parser *parser)
{
    auto *http_request_parser = static_cast<RequestParser *>(parser->data);
    return http_request_parser->onChunkCompleteImpl(parser);
}

int RequestParser::onHeadersCompleteImpl(http_parser*)
{
    const std::optional<HttpMethods> method = to_method(mParser.method);
    if (!method)
    {
        mError = ParseError::MethodNotImplemented;
        return - 1;
    }

    mRequestBuilder.appendMethod(*method);

    if (!flushUrl())
    {
        return -1;
    }
    
    if (!mIsHeaderKey)
    {
        flushHeader();
    }
    
    mRequestBuilder.appendHttpVersion(std::to_string(mParser.http_major) + '.' + std::to_string(mParser.http_minor));
    mState = ParseState::HeadersComplete;
    mRequestBuilder.onHeadersComplete();
    
    const bool chunked = (mParser.flags & F_CHUNKED) != 0;
    const bool hasLength = mParser.content_length != 0 && mParser.content_length != kNoContentLength;    

    if (chunked || hasLength)
    {
        return 1;
    }

    return 0;
}

std::size_t RequestParser::parseHead(std::span<std::byte> input)
{
    if (mError != ParseError::None)
    {
        return 0;
    }

    auto inputPtr = reinterpret_cast<const char *>(input.data());
    std::size_t consumed = http_parser_execute(&mParser, &mSettings, inputPtr, input.size());
    auto code = static_cast<enum http_errno>(mParser.http_errno);

    if (code != HPE_OK && code != HPE_PAUSED && mError == ParseError::None)
    {
        switch (code)
        {
        case HPE_HEADER_OVERFLOW:
            mError = ParseError::HeadersTooLarge;
            break;
        case HPE_INVALID_METHOD:
            mError = ParseError::MethodNotImplemented;
            break;
        default:
            mError = ParseError::BadRequest;
            break;
        }
    }
    return consumed;
}

bool RequestParser::parseComplete() const
{
    return mState == ParseState::Complete;
}

bool RequestParser::parseMessageComplete() const
{
    return mState == ParseState::HeadersComplete;
}

HttpRequest RequestParser::buildRequest() &&
{
    return std::move(mRequestBuilder).buildRequest();
}
} // namespace nebula::http

