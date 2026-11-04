#pragma once

#include "pulsar/result.hpp"
#include "pulsar/task.hpp"

#include <array>
#include <assert.h>
#include <cstddef>
#include <cstring>
#include <span>

namespace pulsar::io
{
template <typename Stream, std::size_t BufferSize = 16 * 1024>
class BufferedReader
{
  public:
    explicit BufferedReader(Stream &stream) : mStream(stream)
    {
    }

    std::span<const std::byte> data() const noexcept;
    std::size_t buffered() const noexcept;
    void consume(std::size_t n) noexcept;

    [[nodiscard]] pulsar::Task<Result<std::size_t>> fill();

    // [[nodiscard]] pulsar::Task<Result<std::span<std::byte>>> peek(std::size_t bytes);
    // [[nodiscard]] pulsar::Task<Result<std::size_t>> discard(std::size_t bytes);
    // [[nodiscard]] pulsar::Task<Result<std::size_t>> readSome(std::span<std::byte> dst);

  private:
    Stream &mStream;
    std::array<std::byte, BufferSize> mBuffer;
    std::size_t mReadSize = 0;
    std::size_t mWriteSize = 0;
};
} // namespace pulsar::io

#include "pulsar/io/impl/buffered_read.ipp"
