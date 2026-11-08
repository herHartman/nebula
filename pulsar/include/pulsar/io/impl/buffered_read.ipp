// buffered_reader.ipp
#ifndef NEBULA_BUFFERED_READER_IPP
#define NEBULA_BUFFERED_READER_IPP

#include "pulsar/io/buffered_reader.hpp"

namespace pulsar::io
{
template <typename Stream, std::size_t BufferSize>
std::span<const std::byte> BufferedReader<Stream, BufferSize>::data() const noexcept
{
    return {mBuffer.data() + mReadSize, mBuffer.data() + mWriteSize};
}

template <typename Stream, std::size_t BufferSize>
std::size_t BufferedReader<Stream, BufferSize>::buffered() const noexcept
{
    return mWriteSize - mReadSize;
}

template <typename Stream, std::size_t BufferSize>
void BufferedReader<Stream, BufferSize>::consume(std::size_t n) noexcept
{
    assert(n <= buffered());
    mReadSize += std::min(n, buffered());
}

template <typename Stream, std::size_t BufferSize>
pulsar::Task<Result<std::size_t>> BufferedReader<Stream, BufferSize>::fill()
{
    if (mReadSize == mWriteSize)
    {
        mWriteSize = 0;
        mReadSize = 0;
    }
    else if (mReadSize > 0)
    {
        const std::size_t buffered = mWriteSize - mReadSize;
        std::memmove(mBuffer.data(), mBuffer.data() + mReadSize, buffered);
        mWriteSize = buffered;
        mReadSize = 0;
    }

    if (mWriteSize >= BufferSize)
    {
        co_return std::unexpected(std::make_error_code(std::errc::no_buffer_space));
    }

    auto result = co_await mStream.read(std::span{mBuffer}.subspan(mWriteSize));
    if (!result.has_value())
    {
        co_return std::unexpected(result.error());
    }

    mWriteSize += *result;
    co_return *result;
}
} // namespace pulsar::io

#endif
