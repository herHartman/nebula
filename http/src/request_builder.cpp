#include "nebula/http/request_builder.hpp"

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

void RequestBuilder::appendQuery(std::unordered_map<std::string, std::string> query)
{
    mQuery = std::move(query);
}

void RequestBuilder::onHeadersComplete()
{
}
} // namespace nebula::http
