#pragma once

#include "nebula/http/http_headers.h"
#include "nebula/http/http_methods.h"
#include "nebula/http/http_request.h"
#include "nebula/http/query_args.hpp"

#include <optional>
#include <string_view>

namespace nebula::http
{
class RequestBuilder
{
  public:
    void appendPath(std::string_view path);

    void appendMethod(HttpMethods method);

    void appendHttpVersion(std::string_view version);

    void appendQuery(QueryArgs query);

    void appendHeader(std::string_view headerFiled, std::string_view headerValue);

    void onHeadersComplete();

    HttpRequest buildRequest() &&;

  private:
    bool mIsComplete = false;
    std::optional<std::string> mPath;
    std::optional<std::string> mHttpVersion;
    std::optional<HttpMethods> mMethod;
    QueryArgs mQuery;
    HttpHeaders mHeaders;
};
} // namespace nebula::http
