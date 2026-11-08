#pragma once

#include "pulsar/io/buffered_reader.hpp"
#include "pulsar/net/tcp.hpp"
#include "pulsar/task.hpp"
#include "pulsar/result.hpp"

#include <cstddef>
#include <vector>

namespace nebula::http
{
template<typename Stream>
class BodyReader
{
  public:
    using Reader = pulsar::io::BufferedReader<Stream>;

    BodyReader(Reader &reader, std::size_t contentLength) noexcept;

    std::size_t remaining() const noexcept;
    std::size_t contentLength() const noexcept;

    [[nodiscard]] pulsar::Task<pulsar::Result<std::size_t>> readSome(std::span<std::byte> buffer);
    [[nodiscard]] pulsar::Task<pulsar::Result<std::vector<std::byte>>> readAll(std::size_t maxSize);
    [[nodiscard]] pulsar::Task<std::error_code> drain();

  private:
    Reader& mReader;
    std::size_t mContentLength = 0;
    std::size_t mConsumed = 0;
};

using TcpBodyReader = BodyReader<pulsar::net::TcpSocket>;
} // namespace nebula::http

#include "nebula/http/impl/body_reader.ipp"
