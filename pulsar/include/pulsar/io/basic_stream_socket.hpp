#pragma once

#include "pulsar/io/basic_socket.hpp"
#include <cstddef>
#include <span>

namespace pulsar::io
{
template <typename Protocol>
class BasicStreamSocket : public BasicSocket<Protocol>
{
  public:
    using BasicSocket<Protocol>::BasicSocket;

    Task<std::size_t> read(std::span<std::byte> buffer)
    {
        int n = co_await IoAwaitable{IoRequest::makeRecv(this->fd(), buffer.size(), buffer.data(), Flags<SendFlags>{})};
        co_return static_cast<std::size_t>(n);
    }

    Task<std::size_t> write(std::span<const std::byte> buffer)
    {
        int n =
            co_await IoAwaitable{IoRequest::makeSend(this->fd(), buffer.size(), const_cast<std::byte *>(buffer.data()),
                                                     Flags<SendFlags>{} | SendFlags::kMsgNosignal)};
        co_return static_cast<std::size_t>(n);
    }

    Task<void> writeAll(std::span<const std::byte> buffer)
    {
        std::size_t written = 0;
        while (written < buffer.size())
        {
            written += co_await write(buffer.subspan(written));
        }
    }
};
} // namespace pulsar::io
