#include "nebula/http/http_request.h"
#include "nebula/http/http_headers.h"
#include "nebula/http/http_methods.h"
#include "nebula/http/query_args.hpp"

namespace nebula::http
{
HttpRequest::HttpRequest(HttpMethods method, HttpHeaders headers, std::string httpVersion, std::string path,
                         QueryArgs queryArgs)
    : mQueryArgs(std::move(queryArgs)), mMethod(method), mHeaders(std::move(headers)),
      mHttpVersion(std::move(httpVersion)), mPath(std::move(path))
{
}

std::string_view HttpRequest::getPath() const noexcept
{
    return mPath;
}

HttpMethods HttpRequest::getMethod() const noexcept
{
    return mMethod;
}

std::string_view HttpRequest::getHttpVersion() const noexcept
{
    return mHttpVersion;
}

std::string_view HttpRequest::getHost() const noexcept
{
    return mHeaders.getHost();
}

} // namespace nebula::http
