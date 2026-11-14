#pragma once

#include "nebula/http/helpers.h"
#include "nebula/http/http_headers.h"
#include "nebula/http/http_methods.h"
#include "nebula/http/query_args.hpp"

#include <nlohmann/json.hpp>

#include <format>
#include <span>
#include <stdexcept>
#include <string>

namespace nebula::http
{
class HttpRequest
{
  public:
    HttpRequest(HttpMethods method, HttpHeaders headers, std::string httpVersion, std::string path,
                QueryArgs queryArgs);

    std::string_view getPath() const noexcept;
    HttpMethods getMethod() const noexcept;
    std::string_view getHttpVersion() const noexcept;
    std::string_view getHost() const noexcept;

    std::optional<std::string_view> findHeader(std::string_view name) const noexcept;
    std::span<const std::string> getHeaders(std::string_view name) const noexcept;

    template <typename T>
    T getQueryArg(std::string_view argName) const;
    template <typename T>
    T getQueryArg(std::string_view argName, const T &defaultParam) const;

  private:
    template <typename T>
    static T convertArg(const QueryArgs &args, std::string_view argName);

    template <typename T>
    static T convertArgOr(const QueryArgs &args, std::string_view argName, const T &defaultParam);

    QueryArgs mQueryArgs;
    HttpMethods mMethod;
    HttpHeaders mHeaders;
    std::string mHttpVersion;
    std::string mPath;
};

template <typename T>
T HttpRequest::convertArg(const QueryArgs &args, std::string_view argName)
{
    const auto it = args.find(argName);
    if (it == args.end())
    {
        throw std::runtime_error(std::format("Arg not found, arg name: {}", argName));
    }

    try
    {
        return helpers::StringTraits<T>::convert(it->second);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error(
            std::format("Exception during parse query, ex was: {}, arg name: {}", e.what(), argName));
    }
}

template <typename T>
T HttpRequest::convertArgOr(const QueryArgs &args, std::string_view argName, const T &defaultParam)
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
T HttpRequest::getQueryArg(std::string_view argName) const
{
    return convertArg<T>(mQueryArgs, argName, "query");
}

template <typename T>
T HttpRequest::getQueryArg(std::string_view argName, const T &defaultParam) const
{
    return convertArgOr<T>(mQueryArgs, argName, defaultParam);
}
} // namespace nebula::http
