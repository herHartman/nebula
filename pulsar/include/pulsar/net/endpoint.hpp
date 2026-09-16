#pragma once

#include <arpa/inet.h>
#include <cstdint>
#include <netinet/in.h>
#include <stdexcept>
#include <string>

namespace pulsar::ip
{
class Endpoint
{
  public:
    Endpoint() noexcept
    {
        mAddr.sin_family = AF_INET;
    }

    static Endpoint v4(const std::string& address, std::uint16_t port)
    {
        Endpoint ep;
        ep.mAddr.sin_port = ::htons(port);
        if (::inet_pton(AF_INET, address.c_str(), &ep.mAddr.sin_addr) != 1)
        {
            throw std::invalid_argument("bad ipv4 address: " + address);
        }
        return ep;
    }

    static Endpoint any(std::uint16_t port) noexcept
    {
        Endpoint ep;
        ep.mAddr.sin_port = ::htons(port);
        ep.mAddr.sin_addr.s_addr = INADDR_ANY;
        return ep;
    }

    static Endpoint fromSockaddr(const ::sockaddr_in& addr) noexcept
    {
        Endpoint ep;
        ep.mAddr = addr;
        return ep;
    }

    std::uint16_t port() const noexcept
    {
        return ::ntohs(mAddr.sin_port);
    }

    std::string address() const
    {
        char buf[INET_ADDRSTRLEN] = {};
        ::inet_ntop(AF_INET, &mAddr.sin_addr, buf, sizeof(buf));
        return buf;
    }

    ::sockaddr_in& raw() noexcept
    {
        return mAddr;
    }

    const ::sockaddr_in& raw() const noexcept
    {
        return mAddr;
    }

  private:
    ::sockaddr_in mAddr{};
};
} // namespace pulsar::ip
