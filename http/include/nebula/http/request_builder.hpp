#pragma once

#include "nebula/http/http_headers.h"
#include "nebula/http/http_methods.h"
#include "nebula/http/http_request.h"

#include <optional>
#include <string_view>
#include <unordered_map>

namespace nebula::http
{
class RequestBuilder
{
  public:
    void appendPath(std::string_view path);

    void appendMethod(HttpMethods method);

    void appendHttpVersion(std::string_view version);

    void appendHeaders(HttpHeaders headers);

    void appendHeaderField(std::string_view headerField);

    void appendHeaderValue(std::string_view headerValue);

    void appendQuery(std::unordered_map<std::string, std::string> query);

    void appendHeader(std::string_view headerFiled, std::string_view headerValue);

    void onHeadersComplete();

    HttpRequest buildRequest() &&;

  private:
    std::optional<std::string> mPath;
    std::optional<std::string> mHttpVersion;
    std::optional<HttpMethods> mMethod;
    std::unordered_map<std::string, std::string> mQuery;
    HttpHeaders mHeaders;
    std::optional<std::string> mCurrentHeaderField;
};
} // namespace nebula::http

