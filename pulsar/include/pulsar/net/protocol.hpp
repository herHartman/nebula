#pragma once

#include "endpoint.hpp"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

namespace pulsar::ip
{
struct tcp
{
    using Endpoint = ip::Endpoint;

    static int family() noexcept
    {
        return AF_INET;
    }

    static int type() noexcept
    {
        return SOCK_STREAM;
    }

    static int protocol() noexcept
    {
        return IPPROTO_TCP;
    }
};

struct udp
{
    using Endpoint = ip::Endpoint;

    static int family() noexcept
    {
        return AF_INET;
    }

    static int type() noexcept
    {
        return SOCK_DGRAM;
    }

    static int protocol() noexcept
    {
        return IPPROTO_UDP;
    }
};
} // namespace pulsar::ip
