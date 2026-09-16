#pragma once

#include "nebula/http/http_headers.h"
#include "nebula/http/http_methods.h"
#include <string>
#include <unordered_map>

#include "nebula/http/helpers.h"
#include "pulsar/task.hpp"

#include <nlohmann/json.hpp>

#include <cstddef>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>

class HttpBodyStreamReader;

namespace nebula::http
{
class HttpRequest
{
  public:
    using QueryArgs = std::unordered_map<std::string, std::string>;
    using PathArgs = std::unordered_map<std::string, std::string>;

    HttpRequest(std::size_t contentLength, HttpMethods method, HttpHeaders headers, std::string host,
                std::string httpVersion, std::shared_ptr<HttpBodyStreamReader> body, std::string path,
                QueryArgs queryArgs, PathArgs pathArgs);

    [[nodiscard]] pulsar::Task<std::string> readAll();

    [[nodiscard]] pulsar::Task<nlohmann::json> readJson();

    template <typename T>
    [[nodiscard]] pulsar::Task<T> readBody();

    [[nodiscard]] std::size_t getContentLength() const noexcept;
    [[nodiscard]] const std::string &getPath() const noexcept;
    [[nodiscard]] HttpMethods getMethod() const noexcept;
    [[nodiscard]] const std::shared_ptr<HttpBodyStreamReader> &getStreamReader() const noexcept;
    [[nodiscard]] const std::string &getHttpVersion() const noexcept;
    [[nodiscard]] const std::string &getHost() const noexcept;

    [[nodiscard]] const std::string *findHeader(const std::string &name) const noexcept;
    [[nodiscard]] std::span<const std::string> getHeaders(const std::string &name) const noexcept;

    template <typename T>
    [[nodiscard]] T getQueryArg(const std::string &argName) const;
    template <typename T>
    [[nodiscard]] T getQueryArg(const std::string &argName, const T &defaultParam) const;

    template <typename T>
    [[nodiscard]] T getPathArg(const std::string &argName) const;
    template <typename T>
    [[nodiscard]] T getPathArg(const std::string &argName, const T &defaultParam) const;

  private:
    template <typename T>
    static T convertArg(const std::unordered_map<std::string, std::string> &args, const std::string &argName,
                        const char *kind);

    template <typename T>
    static T convertArgOr(const std::unordered_map<std::string, std::string> &args, const std::string &argName,
                          const T &defaultParam);

    QueryArgs mQueryArgs;
    PathArgs mPathArgs;
    std::size_t mContentLength;
    HttpMethods mMethod;
    HttpHeaders mHeaders;
    std::shared_ptr<HttpBodyStreamReader> mStreamReader;
    std::string mHost;
    std::string mHttpVersion;
    std::string mPath;
};

template <typename T>
pulsar::Task<T> HttpRequest::readBody()
{
    nlohmann::json body = co_await readJson();
    co_return body.get<T>();
}

template <typename T>
T HttpRequest::convertArg(const std::unordered_map<std::string, std::string> &args, const std::string &argName,
                          const char *kind)
{
    const auto it = args.find(argName);
    if (it == args.end())
    {
        throw std::runtime_error(std::string(kind) + " arg not found: " + argName);
    }

    try
    {
        return helpers::StringTraits<T>::convert(it->second);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(std::string(e.what()) + " (arg: " + argName + ")");
    }
}

template <typename T>
T HttpRequest::convertArgOr(const std::unordered_map<std::string, std::string> &args, const std::string &argName,
                            const T &defaultParam)
{
    const auto it = args.find(argName);
    if (it == args.end())
    {
        return defaultParam;
    }

    try
    {
        return helpers::StringTraits<T>::convert(it->second);
    }
    catch (const std::exception &)
    {
        return defaultParam;
    }
}

template <typename T>
T HttpRequest::getQueryArg(const std::string &argName) const
{
    return convertArg<T>(mQueryArgs, argName, "query");
}

template <typename T>
T HttpRequest::getQueryArg(const std::string &argName, const T &defaultParam) const
{
    return convertArgOr<T>(mQueryArgs, argName, defaultParam);
}

template <typename T>
T HttpRequest::getPathArg(const std::string &argName) const
{
    return convertArg<T>(mPathArgs, argName, "path");
}

template <typename T>
T HttpRequest::getPathArg(const std::string &argName, const T &defaultParam) const
{
    return convertArgOr<T>(mPathArgs, argName, defaultParam);
}
} // namespace nebula::http
