#pragma once

#include "pulsar/io/basic_socket.hpp"
#include "pulsar/io/basic_stream_socket.hpp"
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <system_error>

namespace pulsar::io
{
template <typename Protocol> 
class BasicAcceptor : public BasicSocket<Protocol>
{
  public:
    using BasicSocket<Protocol>::BasicSocket;
    using Endpoint = typename Protocol::Endpoint;

    void listen(int backlog = 128)
    {
        if (::listen(this->fd(), backlog) < 0)
        {
            throw std::system_error(errno, std::system_category(), "listen()");
        }
    }

    Task<BasicStreamSocket<Protocol>> accept()
    {
        ::sockaddr_in addr{};
        ::socklen_t len = sizeof(addr);
        int connFd = co_await IoAwaitable{
            IoRequest::makeAccept(this->fd(), reinterpret_cast<::sockaddr *>(&addr), &len, Flags<AcceptFlags>{})};
        co_return BasicStreamSocket<Protocol>{connFd};
    }
};
} // namespace pulsar::io
