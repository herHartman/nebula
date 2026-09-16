#pragma once

#include "pulsar/io_context.hpp"
#include "pulsar/task.hpp"
#include <cerrno>
#include <system_error>
#include <sys/socket.h>
#include <unistd.h>
#include <utility>

namespace pulsar::io
{
template <typename Protocol>
class BasicSocket
{
  public:
    using Endpoint = typename Protocol::Endpoint;

    BasicSocket() noexcept = default;

    explicit BasicSocket(int fd) noexcept : mFd(fd)
    {
    }

    BasicSocket(const BasicSocket &) = delete;
    BasicSocket &operator=(const BasicSocket &) = delete;

    BasicSocket(BasicSocket &&other) noexcept : mFd(std::exchange(other.mFd, -1))
    {
    }

    BasicSocket &operator=(BasicSocket &&other) noexcept
    {
        if (this != &other)
        {
            closeSync();
            mFd = std::exchange(other.mFd, -1);
        }
        return *this;
    }

    ~BasicSocket()
    {
        closeSync();
    }

    void open(const Protocol &protocol = Protocol())
    {
        closeSync();
        mFd = ::socket(protocol.family(), protocol.type(), protocol.protocol());
        if (mFd < 0)
        {
            throw std::system_error(errno, std::system_category(), "socket()");
        }
    }

    void bind(const Endpoint &endpoint)
    {
        const auto &addr = endpoint.raw();
        if (::bind(mFd, reinterpret_cast<const ::sockaddr *>(&addr), sizeof(addr)) < 0)
        {
            throw std::system_error(errno, std::system_category(), "bind()");
        }
    }

    void setReuseAddr(bool enable = true) noexcept
    {
        int value = enable ? 1 : 0;
        ::setsockopt(mFd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
    }

    Task<void> connect(Endpoint peerEndpoint)
    {
        auto &addr = peerEndpoint.raw();
        co_await IoAwaitable{IoRequest::makeConnect(mFd, reinterpret_cast<::sockaddr *>(&addr), sizeof(addr))};
    }

    Task<void> close()
    {
        if (mFd >= 0)
        {
            int fd = std::exchange(mFd, -1);
            co_await IoAwaitable{IoRequest::makeClose(fd)};
        }
    }

    int fd() const noexcept
    {
        return mFd;
    }

    bool isOpen() const noexcept
    {
        return mFd >= 0;
    }

  protected:
    void closeSync() noexcept
    {
        if (mFd >= 0)
        {
            ::close(mFd);
            mFd = -1;
        }
    }

    int mFd = -1;
};
} // namespace pulsar::io
