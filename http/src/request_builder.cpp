#include "nebula/http/request_builder.hpp"
#include "nebula/http/http_request.h"
#include <stdexcept>

namespace nebula::http
{
void RequestBuilder::appendPath(std::string_view path)
{
    mPath = path;
}

void RequestBuilder::appendMethod(HttpMethods method)
{
    mMethod = method;
}

void RequestBuilder::appendHttpVersion(std::string_view version)
{
    mHttpVersion = version;
}

void RequestBuilder::appendHeader(std::string_view headerFiled, std::string_view headerValue)
{
    mHeaders.add(std::string{headerFiled}, std::string{headerValue});
}

void RequestBuilder::appendQuery(QueryArgs query)
{
    mQuery = std::move(query);
}

void RequestBuilder::onHeadersComplete()
{
    mIsComplete = true;
}

HttpRequest RequestBuilder::buildRequest() &&
{
    if (!mMethod)
    {
        throw std::runtime_error("method is requred");
    }

    if (!mPath)
    {
        throw std::runtime_error("path is requred");
    }

    if (!mHttpVersion)
    {
        throw std::runtime_error("version is requred");
    }

    return HttpRequest(*mMethod, std::move(mHeaders), std::move(*mHttpVersion), std::move(*mPath), std::move(mQuery));
}
} // namespace nebula::http
