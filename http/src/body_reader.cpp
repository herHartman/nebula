#include "nebula/http/body_reader.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <system_error>
#include <vector>

namespace nebula::http
{
BodyReader::BodyReader(Reader &reader, std::size_t contentLength) noexcept
    : mReader(reader), mContentLength(contentLength)
{
}

std::size_t BodyReader::remaining() const noexcept
{
    return mContentLength - mConsumed;
}

std::size_t BodyReader::contentLength() const noexcept
{
    return mContentLength;
}

[[nodiscard]] pulsar::Task<BodyReader::Result<std::size_t>> BodyReader::readSome(std::span<std::byte> buffer)
{
    if (remaining() == 0)
    {
        co_return 0;
    }

    if (buffer.empty())
    {
        co_return 0;
    }

    if (!mReader.buffered())
    {
        Result fillResult = co_await mReader.fill();
        if (!fillResult.has_value())
        {
            co_return std::unexpected(fillResult.error());
        }
        else if (!fillResult.value())
        {
            co_return std::unexpected(std::make_error_code(std::errc::connection_aborted));
        }
    }

    const std::size_t readSize = std::min({mReader.buffered(), buffer.size(), remaining()});
    std::ranges::copy(mReader.data().first(readSize), buffer.begin());
    mReader.consume(readSize);
    mConsumed += readSize;
    co_return readSize;
}

[[nodiscard]] pulsar::Task<BodyReader::Result<std::vector<std::byte>>> BodyReader::readAll(std::size_t maxSize)
{
    if (remaining() > maxSize)
    {
        co_return std::unexpected(std::make_error_code(std::errc::value_too_large));
    }

    std::vector<std::byte> outputBuffer;
    outputBuffer.resize(maxSize);

    std::size_t readSize = 0;
    while (remaining())
    {
        auto outputView = std::span<std::byte>{outputBuffer}.subspan(readSize);
        Result result = co_await readSome(outputView);
        if (!result.has_value())
        {
            co_return std::unexpected(result.error());
        }
        else if (result.value() == 0)
        {
            co_return std::make_error_code(std::errc::connection_aborted);
        }
        readSize += result.value();
    }

    co_return outputBuffer;
}

[[nodiscard]] pulsar::Task<std::error_code> BodyReader::drain()
{
    std::array<std::byte, 16384> buffer;
    static constexpr std::size_t kMaxDrainSize = 256 * 1024;
    std::size_t drained = 0;

    while (remaining())
    {
        if (drained > kMaxDrainSize)
        {
            co_return std::make_error_code(std::errc::message_size);
        }

        Result result = co_await readSome(buffer);
        if (!result.has_value())
        {
            co_return result.error();
        }
        else if (result.value() == 0)
        {
            co_return std::make_error_code(std::errc::connection_aborted);
        }

        drained += result.value();
    }
    co_return std::error_code{};
}
} // namespace nebula::http
