#pragma once

#include "enum_flags.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <liburing/io_uring.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>

namespace pulsar
{
enum class SendFlags
{
    kNoSet = 0,
    kMsgConfirm = MSG_CONFIRM,
    kMsgDontroute = MSG_DONTROUTE,
    kMsgDontwait = MSG_DONTWAIT,
    kMsgEor = MSG_EOR,
    kMsgMore = MSG_MORE,
    kMsgNosignal = MSG_NOSIGNAL,
    kMsgOOB = MSG_OOB,
    kMsgFastopen = MSG_FASTOPEN,
};

enum class AcceptFlags
{
    kNosSet = 0,
    kSockNonblock = SOCK_NONBLOCK,
    KSockCloexec = SOCK_CLOEXEC,
};

struct IoRequest
{
    enum class Type : std::uint8_t
    {
        kRead,
        kReadv,
        kWrite,
        kWritev,
        kFsync,
        kFdatasync,
        kRecv,
        kRecvmsg,
        kSend,
        kSendmsg,
        kAccept,
        kConnect,
        kClose,
        kPoolAdd,
        kPoolRemove,
        kCancel,
    };

    struct Read
    {
        Type type;
        int fd;
        std::size_t length;
        std::byte *buffer;
        std::uint64_t offset;
    };

    struct Write
    {
        Type type;
        int fd;
        std::size_t length;
        std::byte *buffer;
        std::uint64_t offset;
    };

    struct Readv
    {
        Type type;
        int fd;
        const struct ::iovec *iovecs;
        std::size_t iovecLength;
        std::uint64_t offset;
    };

    struct Writev
    {
        Type type;
        int fd;
        const struct ::iovec *iovecs;
        std::size_t iovecLength;
        std::uint64_t offset;
    };

    struct Fsync
    {
        Type type;
        int fd;
    };

    struct Close
    {
        Type type;
        int fd;
    };

    struct Send
    {
        Type type;
        int fd;
        std::size_t length;
        std::byte *buffer;
        Flags<SendFlags> flags;
    };

    struct Accept
    {
        Type type;
        int fd;
        struct ::sockaddr *addr;
        ::socklen_t *addrlen;
        Flags<AcceptFlags> flags;
    };

    struct Connect
    {
        Type type;
        int fd;
        struct ::sockaddr *addr;
        ::socklen_t addrlen;
    };

    struct Recv
    {
        Type type;
        int fd;
        std::size_t length;
        std::byte *buffer;
        Flags<SendFlags> flags;
    };

    struct Sendmsg
    {
        Type type;
        int fd;
        const struct ::msghdr *msg;
        Flags<SendFlags> flags;
    };

    struct Recvmsg
    {
        Type type;
        int fd;
        struct ::msghdr *msg;
        Flags<SendFlags> flags;
    };

    union {
        Read read;
        Readv readv;
        Write write;
        Writev writev;
        Fsync fsync;
        Close close;
        Send send;
        Accept accept;
        Connect connect;
        Recv recv;
        Sendmsg sendmsg;
        Recvmsg recvmsg;
    };

    constexpr IoRequest(Read read) : read(read)
    {
    }

    constexpr IoRequest(Write write) : write(write)
    {
    }

    constexpr IoRequest(Readv readv) : readv(readv)
    {
    }

    constexpr IoRequest(Writev writev) : writev(writev)
    {
    }

    constexpr IoRequest(Fsync fsync) : fsync(fsync)
    {
    }

    constexpr IoRequest(Close close) : close(close)
    {
    }

    constexpr IoRequest(Send send) : send(send)
    {
    }

    constexpr IoRequest(Accept accept) : accept(accept)
    {
    }

    constexpr IoRequest(Connect connect) : connect(connect)
    {
    }

    constexpr IoRequest(Recv recv) : recv(recv)
    {
    }

    constexpr IoRequest(Sendmsg sendmsg) : sendmsg(sendmsg)
    {
    }

    constexpr IoRequest(Recvmsg recvmsg) : recvmsg(recvmsg)
    {
    }

    constexpr static IoRequest makeRead(int fd, std::size_t length, std::byte *buffer, std::size_t offset)
    {
        return Read{.type = Type::kRead, .fd = fd, .length = length, .buffer = buffer, .offset = offset};
    }

    constexpr static IoRequest makeWrite(int fd, std::size_t length, std::byte *buffer, std::size_t offset)
    {
        return Write{.type = Type::kWrite, .fd = fd, .length = length, .buffer = buffer, .offset = offset};
    }

    constexpr static IoRequest makeReadv(int fd, const struct ::iovec *iovecs, std::size_t iovecLength,
                                         std::size_t offset)
    {
        return Readv{.type = Type::kReadv, .fd = fd, .iovecs = iovecs, .iovecLength = iovecLength, .offset = offset};
    }

    constexpr static IoRequest makeWritev(int fd, const struct ::iovec *iovecs, std::size_t iovecLength,
                                          std::size_t offset)
    {
        return Writev{.type = Type::kWritev, .fd = fd, .iovecs = iovecs, .iovecLength = iovecLength, .offset = offset};
    }

    constexpr static IoRequest makeClose(int fd)
    {
        return Close{.type = Type::kClose, .fd = fd};
    }

    constexpr static IoRequest makeFsync(int fd)
    {
        return Fsync{.type = Type::kFsync, .fd = fd};
    }

    constexpr static IoRequest makeSend(int fd, std::size_t length, std::byte *buffer, Flags<SendFlags> flags)
    {
        return Send{.type = Type::kSend, .fd = fd, .length = length, .buffer = buffer, .flags = flags};
    }

    constexpr static IoRequest makeAccept(int fd, struct ::sockaddr *addr, ::socklen_t *addrlen,
                                          Flags<AcceptFlags> flags)
    {
        return Accept{.type = Type::kAccept, .fd = fd, .addr = addr, .addrlen = addrlen, .flags = flags};
    }

    constexpr static IoRequest makeConnect(int fd, struct ::sockaddr *addr, ::socklen_t addrlen)
    {
        return Connect{.type = Type::kConnect, .fd = fd, .addr = addr, .addrlen = addrlen};
    }

    constexpr static IoRequest makeRecv(int fd, std::size_t length, std::byte *buffer, Flags<SendFlags> flags)
    {
        return Recv{.type = Type::kRecv, .fd = fd, .length = length, .buffer = buffer, .flags = flags};
    }

    constexpr static IoRequest makeSendmsg(int fd, const struct ::msghdr *msg, Flags<SendFlags> flags)
    {
        return Sendmsg{.type = Type::kSendmsg, .fd = fd, .msg = msg, .flags = flags};
    }

    constexpr static IoRequest makeRecvmsg(int fd, struct ::msghdr *msg, Flags<SendFlags> flags)
    {
        return Recvmsg{.type = Type::kRecvmsg, .fd = fd, .msg = msg, .flags = flags};
    }

    constexpr Type type() const noexcept
    {
        return read.type;
    }

    template <Type>
    struct member;

    template <Type T>
    constexpr auto &as() noexcept
    {
        return this->*member<T>::ptr;
    }

    template <Type T>
    constexpr const auto &as() const noexcept
    {
        return this->*member<T>::ptr;
    }
};

template <>
struct IoRequest::member<IoRequest::Type::kRead>
{
    static constexpr auto ptr = &IoRequest::read;
};

template <>
struct IoRequest::member<IoRequest::Type::kReadv>
{
    static constexpr auto ptr = &IoRequest::readv;
};

template <>
struct IoRequest::member<IoRequest::Type::kWrite>
{
    static constexpr auto ptr = &IoRequest::write;
};

template <>
struct IoRequest::member<IoRequest::Type::kWritev>
{
    static constexpr auto ptr = &IoRequest::writev;
};

template <>
struct IoRequest::member<IoRequest::Type::kFsync>
{
    static constexpr auto ptr = &IoRequest::fsync;
};

template <>
struct IoRequest::member<IoRequest::Type::kClose>
{
    static constexpr auto ptr = &IoRequest::close;
};

template <>
struct IoRequest::member<IoRequest::Type::kSend>
{
    static constexpr auto ptr = &IoRequest::send;
};

template <>
struct IoRequest::member<IoRequest::Type::kAccept>
{
    static constexpr auto ptr = &IoRequest::accept;
};

template <>
struct IoRequest::member<IoRequest::Type::kConnect>
{
    static constexpr auto ptr = &IoRequest::connect;
};

template <>
struct IoRequest::member<IoRequest::Type::kRecv>
{
    static constexpr auto ptr = &IoRequest::recv;
};

template <>
struct IoRequest::member<IoRequest::Type::kSendmsg>
{
    static constexpr auto ptr = &IoRequest::sendmsg;
};

template <>
struct IoRequest::member<IoRequest::Type::kRecvmsg>
{
    static constexpr auto ptr = &IoRequest::recvmsg;
};

static_assert(std::is_trivially_copyable_v<IoRequest>);
static_assert(std::is_trivially_destructible_v<IoRequest>);

static_assert(std::is_trivially_copyable_v<Flags<SendFlags>>);
static_assert(std::is_trivially_copyable_v<Flags<AcceptFlags>>);
} // namespace pulsar
