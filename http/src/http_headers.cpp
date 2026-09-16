#include "nebula/http/http_headers.h"
#include <format>
#include <ranges>
#include <unordered_set>

namespace nebula::http
{
HttpHeaders::Iterator HttpHeaders::begin() noexcept
{
    return mHeaders.begin();
}

HttpHeaders::Iterator HttpHeaders::end() noexcept
{
    return mHeaders.end();
}

HttpHeaders::ConstIterator HttpHeaders::begin() const noexcept
{
    return mHeaders.begin();
}

HttpHeaders::ConstIterator HttpHeaders::end() const noexcept
{
    return mHeaders.end();
}

HttpHeaders::ConstIterator HttpHeaders::cbegin() const noexcept
{
    return mHeaders.cbegin();
}
HttpHeaders::ConstIterator HttpHeaders::cend() const noexcept
{
    return mHeaders.cend();
}

HttpHeaders::Iterator HttpHeaders::find(const HeaderKey &key)
{
    return mHeaders.find(key);
}

HttpHeaders::ConstIterator HttpHeaders::find(const HeaderKey &key) const
{
    return mHeaders.find(key);
}

HeaderValues &HttpHeaders::headers() noexcept
{
    return mHeaders;
}

const HeaderValues &HttpHeaders::headers() const noexcept
{
    return mHeaders;
}

bool HttpHeaders::empty() const noexcept
{
    return mHeaders.empty();
}

size_t HttpHeaders::size() const noexcept
{
    return mHeaders.size();
}

HttpHeaders::Iterator HttpHeaders::erase(ConstIterator pos)
{
    return mHeaders.erase(pos);
}

size_t HttpHeaders::erase(const HeaderKey &key)
{
    return mHeaders.erase(key);
}

void HttpHeaders::clear() noexcept
{
    mHeaders.clear();
}

void HttpHeaders::set(const HeaderKey &key, HeaderValue value)
{
    mHeaders[key] = {std::move(value)};
}

void HttpHeaders::set(const HeaderKey &key, std::vector<HeaderValue> values)
{
    mHeaders[key] = std::move(values);
}

void HttpHeaders::add(const HeaderKey &key, HeaderValue value)
{
    auto entry = mHeaders.find(key);
    if (entry == mHeaders.end())
    {
        set(key, std::move(value));
    }
    else
    {
        entry->second.emplace_back(std::move(value));
    }
}

void HttpHeaders::add(const HeaderKey &key, std::vector<HeaderValue> values)
{
    auto entry = mHeaders.find(key);
    if (entry == mHeaders.end())
    {
        set(key, std::move(values));
    }
    else
    {
        for (auto iter : values)
        {
            entry->second.emplace_back(std::move(iter));
        }
    }
}

void HttpHeaders::setBasicAuth(std::string username, std::string password)
{
}

void HttpHeaders::setBearerAuth(std::string token)
{
}

void HttpHeaders::setConnection(std::string connection)
{
}

void HttpHeaders::setContentLength(std::size_t contentSize)
{
}

void HttpHeaders::setContentType(std::string mediaType)
{
}

void HttpHeaders::setDate(std::string date)
{
}

void HttpHeaders::setHost(std::string host)
{
}

void HttpHeaders::setLocation(std::string location)
{
}

void HttpHeaders::setOrigin(std::string origin)
{
}

std::vector<HeaderValue> &HttpHeaders::getHeaderValues(const HeaderKey &key)
{
    return mHeaders[key];
}

size_t HttpHeaders::getContentLength() const
{
    const auto &content_length_vec = mHeaders.at(BasicHeaders::kContentLength);
    if (!content_length_vec.empty())
    {
        auto &content_legnth_str = content_length_vec.back();
        return std::stoull(content_legnth_str);
    }
    return 0;
}

const std::string &HttpHeaders::getContentType() const
{
}

const std::string &HttpHeaders::getHost() const
{
    const auto &host_vec = mHeaders.at(BasicHeaders::kHost);
    return host_vec.back();
}

bool HttpHeaders::canCombineHeaders(const std::string &headerKey)
{
    static const std::unordered_set<std::string> nonCominableHeaders = {BasicHeaders::kSetCookie,
                                                                        BasicHeaders::kWWWAuthenticate,
                                                                        BasicHeaders::kProxyAuthenticate,
                                                                        BasicHeaders::kLink,
                                                                        BasicHeaders::kVary,
                                                                        BasicHeaders::kStrictTransportSecurity};

    return nonCominableHeaders.find(headerKey) == nonCominableHeaders.end();
}

std::string HttpHeaders::formatHeaders() const
{
    std::size_t reserve = 2;
    for (const auto &[name, values] : mHeaders)
    {
        for (const auto &value : values)
        {
            reserve += name.size() + value.size() + 4;
        }
    }

    std::string out;
    out.reserve(reserve);
    auto it = std::back_inserter(out);

    for (const auto &[name, values] : mHeaders | std::views::filter([](const auto &e) { return !e.second.empty(); }))
    {
        if (canCombineHeaders(name))
        {
            std::format_to(it, "{}: ", name);
            bool first = true;
            for (const auto &value : values)
            {
                if (!first)
                {
                    out += ", ";
                }
                first = false;
                out += value;
            }
            out += "\r\n";
        }
        else
        {
            for (const auto &value : values)
            {
                std::format_to(it, "{}: {}\r\n", name, value);
            }
        }
    }

    out += "\r\n";
    return out;
}

void HttpHeaders::addHeaders(HttpHeaders headers)
{
    mHeaders.insert(std::make_move_iterator(headers.begin()), std::make_move_iterator(headers.end()));
}

bool HttpHeaders::hasContentLength() const
{
    return mHeaders.find(BasicHeaders::kContentLength) != mHeaders.end();
}
} // namespace nebula::http