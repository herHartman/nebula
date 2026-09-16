#pragma once

#include "pulsar/io/basic_socket.hpp"
#include <cstddef>
#include <netinet/in.h>
#include <span>
#include <sys/socket.h>
#include <sys/uio.h>

namespace pulsar::io
{
template <typename Protocol>
class BasicDatagramSocket : public BasicSocket<Protocol>
{
  public:
    using BasicSocket<Protocol>::BasicSocket;
    using Endpoint = typename Protocol::Endpoint;

    struct Received
    {
        std::size_t size;
        Endpoint from;
    };

    Task<std::size_t> sendTo(std::span<const std::byte> buffer, Endpoint to)
    {
        ::iovec iov{.iov_base = const_cast<std::byte *>(buffer.data()), .iov_len = buffer.size()};
        ::msghdr msg{};
        msg.msg_name = &to.raw();
        msg.msg_namelen = sizeof(to.raw());
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        int n = co_await IoAwaitable{IoRequest::makeSendmsg(this->fd(), &msg, Flags<SendFlags>{})};
        co_return static_cast<std::size_t>(n);
    }

    Task<Received> receiveFrom(std::span<std::byte> buffer)
    {
        ::sockaddr_in fromAddr{};
        ::iovec iov{.iov_base = buffer.data(), .iov_len = buffer.size()};
        ::msghdr msg{};
        msg.msg_name = &fromAddr;
        msg.msg_namelen = sizeof(fromAddr);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;

        int n = co_await IoAwaitable{IoRequest::makeRecvmsg(this->fd(), &msg, Flags<SendFlags>{})};
        co_return Received{.size = static_cast<std::size_t>(n), .from = Endpoint::fromSockaddr(fromAddr)};
    }
};
} // namespace pulsar::io
