
#include "http_parser.h"
#include "nebula/http/request_builder.hpp"
#include <cstddef>

namespace nebula::http
{
struct ParserLimits
{
    std::size_t maxTargetSize = 8 * 1024;
    std::size_t maxHeadersSize = 16 * 1024;
};

enum class ParseError : std::uint8_t
{
    None,
    // Нарушен синтаксис HTTP/1.x -> 400
    BadRequest,
    // Request target длиннее лимита -> 414
    UriTooLong,
    // Заголовки больше лимита -> 431
    HeadersTooLarge,
    // Метод неизвестен или не поддерживается -> 501
    MethodNotImplemented,
    // Версия протокола не 1.x -> 505
    VersionNotSupported,
};

class RequestParser
{
  public:
    explicit RequestParser(ParserLimits limits = {});

    int onMessageBeginImpl(http_parser *parser);
    int onUrlImpl(http_parser *parser, const char *data, std::size_t lenght);
    int onStatusImpl(http_parser *parser, const char *data, std::size_t lenght);
    int onHeaderFieldImpl(http_parser *parser, const char *data, std::size_t lenght);
    int onHeaderValueImpl(http_parser *parser, const char *data, std::size_t lenght);
    int onHeadersCompleteImpl(http_parser *parser);
    int onMessageBodyImpl(http_parser *parser, const char *data, std::size_t lenght);
    int onMessageCompleteImpl(http_parser *parser);

    int onChunkHeaderImpl(http_parser *parser);
    int onChunkCompleteImpl(http_parser *parser);

    static int onMessageBegin(http_parser *parser);
    static int onUrl(http_parser *parser, const char *data, std::size_t lenght);
    static int onStatus(http_parser *parser, const char *data, std::size_t lenght);
    static int onHeaderField(http_parser *parser, const char *data, std::size_t lenght);
    static int onHeaderValue(http_parser *parser, const char *data, std::size_t lenght);
    static int onHeadersComplete(http_parser *parser);
    static int onMessageBody(http_parser *parser, const char *data, std::size_t lenght);
    static int onMessageComplete(http_parser *parser);

    static int onChunkHeader(http_parser *parser);
    static int onChunkComplete(http_parser *parser);

    std::size_t parseHead(std::span<std::byte> input);

    bool parseComplete() const;
    bool parseMessageComplete() const;

    HttpRequest buildRequest() &&;

    ParseError fail() const;

    RequestParser(RequestParser &) = delete;
    RequestParser(RequestParser &&) = delete;
    RequestParser &operator=(const RequestParser &) = delete;
    RequestParser &operator=(RequestParser &&) = delete;

  private:
    static constexpr std::size_t kMaxUrlSize = 8 * 1024;
    static constexpr std::size_t kMaxHeaderSize = 8 * 1024;
    static constexpr std::size_t kMaxHeadersSize = 16 * 1024;
    static constexpr std::size_t kMaxHeadersCount = 100;

    enum class ParseState : std::uint8_t
    {
        Inited,
        InProgress,
        HeadersComplete,
        Complete,
        Error,
    };

    enum class ParseQueryState : std::uint8_t
    {
        Value,
        Name,
    };

    bool flushUrl();
    void flushHeader();

    static const http_parser_settings mSettings;
    http_parser mParser;
    http_parser_url mUrlParser;
    RequestBuilder mRequestBuilder;
    ParseState mState;
    ParseError mError = ParseError::None;
    ParserLimits mLimits;
    std::string mUrl;
    std::string mHeaderKey;
    std::string mHeaderValue;
    std::size_t mHeaderSize = 0;
    std::size_t mHeadersSize = 0;
    bool mIsHeaderKey = true;
};
} // namespace nebula::http
