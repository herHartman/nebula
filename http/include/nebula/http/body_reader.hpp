#pragma once

#include "pulsar/io/buffered_reader.hpp"
#include "pulsar/net/tcp.hpp"
#include "pulsar/task.hpp"
#include <cstddef>
#include <expected>

namespace nebula::http
{
class BodyReader
{
  public:
    template <typename T>
    using Result = std::expected<T, std::error_code>;

    using Reader = pulsar::io::BufferedReader<pulsar::net::TcpSocket>;

    BodyReader(Reader &reader, std::size_t contentLength) noexcept;

    std::size_t remaining() const noexcept;
    std::size_t contentLength() const noexcept;

    [[nodiscard]] pulsar::Task<Result<std::size_t>> readSome(std::span<std::byte> buffer);
    [[nodiscard]] pulsar::Task<Result<std::vector<std::byte>>> readAll(std::size_t maxSize);
    [[nodiscard]] pulsar::Task<Result<void>> drain();

  private:
    Reader *mReader = nullptr;
    std::size_t mContentLength = 0;
    std::size_t mReadSize = 0;
};
} // namespace nebula::http
