#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace nebula::http
{
using HeaderValue = std::string;
using HeaderKey = std::string;

struct StringHash
{
    using is_transparent = void;

    std::size_t operator()(std::string_view sv) const
    {
        return std::hash<std::string_view>{}(sv);
    }

    std::size_t operator()(const std::string &s) const
    {
        return std::hash<std::string>{}(s);
    }

    std::size_t operator()(const char *str) const
    {
        return std::hash<std::string_view>{}(str);
    }
};

using HeaderValues = std::unordered_map<HeaderKey, std::vector<HeaderValue>, StringHash, std::equal_to<>>;
class HttpHeaders
{
  public:
    using Iterator = HeaderValues::iterator;
    using ConstIterator = HeaderValues::const_iterator;

    using iterator = Iterator;
    using const_iterator = ConstIterator;

    Iterator begin() noexcept;
    Iterator end() noexcept;

    ConstIterator begin() const noexcept;
    ConstIterator end() const noexcept;

    ConstIterator cbegin() const noexcept;
    ConstIterator cend() const noexcept;

    Iterator find(const HeaderKey &key);
    ConstIterator find(const HeaderKey &key) const;

    HeaderValues &headers() noexcept;
    const HeaderValues &headers() const noexcept;

    bool empty() const noexcept;
    std::size_t size() const noexcept;

    Iterator erase(ConstIterator pos);
    std::size_t erase(const HeaderKey &key);
    void clear() noexcept;

    struct BasicHeaders
    {
        static constexpr char kAccept[] = "Accept";
        static constexpr char kAcceptCharset[] = "Accept-Charset";
        static constexpr char kAcceptEncoding[] = "Accept-Encoding";
        static constexpr char kacceptLanguage[] = "Accept-Language";
        static constexpr char kAcceptRanges[] = "Accept-Ranges";
        static constexpr char kAge[] = "Age";
        static constexpr char kAllow[] = "Allow";
        static constexpr char kAlternates[] = "Alternates";
        static constexpr char kAuthorization[] = "Authorization";
        static constexpr char kCache_control[] = "Cache-Control";
        static constexpr char kConnection[] = "Connection";
        static constexpr char kContentLength[] = "Content-Length";
        static constexpr char kHost[] = "Host";
        static constexpr char kSetCookie[] = "Set-Cookie";
        static constexpr char kWWWAuthenticate[] = "WWW-Authenticate";
        static constexpr char kProxyAuthenticate[] = "Proxy-Authenticate";
        static constexpr char kLink[] = "Link";
        static constexpr char kVary[] = "Vary";
        static constexpr char kStrictTransportSecurity[] = "Strict-Transport-Security";
    };

    void set(const HeaderKey &key, HeaderValue value);
    void set(const HeaderKey &key, std::vector<HeaderValue> values);

    void add(const HeaderKey &key, HeaderValue value);
    void add(const HeaderKey &key, std::vector<HeaderValue> values);

    void setBasicAuth(std::string username, std::string password);
    void setBearerAuth(std::string token);
    void setConnection(std::string connection);
    void setContentLength(std::size_t contentSize);
    void setContentType(std::string mediaType);
    void setDate(std::string date);
    void setHost(std::string host);
    void setLocation(std::string location);
    void setOrigin(std::string origin);

    std::size_t getContentLength() const;
    const std::string &getHost() const;

    std::vector<HeaderValue> &getHeaderValues(const HeaderKey &key);
    std::string formatHeaders() const;

    const std::string &getContentType() const;

    void addHeaders(HttpHeaders headers);
    bool hasContentLength() const;

  private:
    static bool canCombineHeaders(const std::string &headerKey);

    HeaderValues mHeaders{};
};
} // namespace nebula::http
