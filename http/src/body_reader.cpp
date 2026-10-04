#include "nebula/http/body_reader.hpp"
#include <cstddef>
#include <vector>

namespace nebula::http
{
BodyReader::BodyReader(Reader &reader, std::size_t contentLength) noexcept
    : mReader(&reader), mContentLength(contentLength)
{
}

std::size_t BodyReader::remaining() const noexcept
{
    return mContentLength - mReadSize;
}

std::size_t BodyReader::contentLength() const noexcept
{
    return mContentLength;
}

[[nodiscard]] pulsar::Task<BodyReader::Result<std::size_t>> BodyReader::readSome(std::span<std::byte> buffer)
{
    
}

[[nodiscard]] pulsar::Task<BodyReader::Result<std::vector<std::byte>>> BodyReader::readAll(std::size_t maxSize)
{
}

[[nodiscard]] pulsar::Task<BodyReader::Result<void>> drain()
{
}
} // namespace nebula::http
