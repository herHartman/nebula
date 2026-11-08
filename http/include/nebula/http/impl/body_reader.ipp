// body_reader.ipp
#ifndef NEBULA_BODY_READER_IPP
#define NEBULA_BODY_READER_IPP

#include "nebula/http/body_reader.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <system_error>

namespace nebula::http
{
template<typename Stream>
BodyReader<Stream>::BodyReader(BodyReader<Stream>::Reader &reader, std::size_t contentLength) noexcept
    : mReader(reader), mContentLength(contentLength)
{
}

template<typename Stream>
std::size_t BodyReader<Stream>::remaining() const noexcept
{
    return mContentLength - mConsumed;
}

template<typename Stream>
std::size_t BodyReader<Stream>::contentLength() const noexcept
{
    return mContentLength;
}

template<typename Stream>
auto BodyReader<Stream>::readSome(std::span<std::byte> buffer) -> pulsar::Task<pulsar::Result<std::size_t>>
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
        pulsar::Result fillResult = co_await mReader.fill();
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

template<typename Stream>
auto BodyReader<Stream>::readAll(std::size_t maxSize) -> pulsar::Task<pulsar::Result<std::vector<std::byte>>>
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
        pulsar::Result result = co_await readSome(outputView);
        if (!result.has_value())
        {
            co_return std::unexpected(result.error());
        }
        else if (result.value() == 0)
        {
            co_return std::unexpected(std::make_error_code(std::errc::connection_aborted));
        }
        readSize += result.value();
    }

    co_return outputBuffer;
}

template<typename Stream>
[[nodiscard]] pulsar::Task<std::error_code> BodyReader<Stream>::drain()
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

        pulsar::Result result = co_await readSome(buffer);
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

#endif

