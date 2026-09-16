#include "test_support.hpp"

#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

namespace test_support
{
std::uint16_t freePort(int sockType)
{
    const int fd = ::socket(AF_INET, sockType, 0);
    if (fd < 0)
    {
        throw std::runtime_error("socket() failed");
    }

    ::sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;

    if (::bind(fd, reinterpret_cast<::sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        ::close(fd);
        throw std::runtime_error("bind() failed");
    }

    ::socklen_t len = sizeof(addr);
    if (::getsockname(fd, reinterpret_cast<::sockaddr*>(&addr), &len) < 0)
    {
        ::close(fd);
        throw std::runtime_error("getsockname() failed");
    }

    ::close(fd);
    return ntohs(addr.sin_port);
}
} // namespace test_support
