#include "pulsar/uring_backend.hpp"
#include "pulsar/io_completion.h"
#include "pulsar/io_operation.h"
#include <cerrno>
#include <liburing.h>
#include <liburing/io_uring.h>
#include <system_error>

namespace pulsar::io_engine
{
namespace
{
void prepare_sqe(io_uring_sqe *sqe, const IoRequest &req, IoCompletion *completion)
{
    using Type = IoRequest::Type;

    switch (req.type())
    {
    case Type::kRead: {
        const auto &op = req.as<Type::kRead>();
        ::io_uring_prep_read(sqe, op.fd, op.buffer, op.length, op.offset);
        break;
    }

    case Type::kWrite: {
        const auto &op = req.as<Type::kWrite>();
        ::io_uring_prep_write(sqe, op.fd, op.buffer, op.length, op.offset);
        break;
    }

    case Type::kReadv: {
        const auto &op = req.as<Type::kReadv>();
        ::io_uring_prep_readv(sqe, op.fd, op.iovecs, op.iovecLength, op.offset);
        break;
    }

    case Type::kWritev: {
        const auto &op = req.as<Type::kWritev>();
        ::io_uring_prep_writev(sqe, op.fd, op.iovecs, op.iovecLength, op.offset);
        break;
    }

    case Type::kFsync: {
        const auto &op = req.as<Type::kFsync>();
        ::io_uring_prep_fsync(sqe, op.fd,
                              0); // 0 = full fsync, не IORING_FSYNC_DATASYNC
        break;
    }

    case Type::kSend: {
        const auto &op = req.as<Type::kSend>();
        ::io_uring_prep_send(sqe, op.fd, op.buffer, op.length, static_cast<int>(op.flags.get()));
        break;
    }

    case Type::kRecv: {
        const auto &op = req.as<Type::kRecv>();
        ::io_uring_prep_recv(sqe, op.fd, op.buffer, op.length, static_cast<int>(op.flags.get()));
        break;
    }

    case Type::kSendmsg: {
        const auto &op = req.as<Type::kSendmsg>();
        ::io_uring_prep_sendmsg(sqe, op.fd, op.msg, static_cast<int>(op.flags.get()));
        break;
    }

    case Type::kRecvmsg: {
        const auto &op = req.as<Type::kRecvmsg>();
        ::io_uring_prep_recvmsg(sqe, op.fd, op.msg, static_cast<int>(op.flags.get()));
        break;
    }

    case Type::kAccept: {
        const auto &op = req.as<Type::kAccept>();
        ::io_uring_prep_accept(sqe, op.fd, op.addr, op.addrlen, static_cast<int>(op.flags.get()));
        break;
    }

    case Type::kConnect: {
        const auto &op = req.as<Type::kConnect>();
        ::io_uring_prep_connect(sqe, op.fd, op.addr, op.addrlen);
        break;
    }

    case Type::kClose: {
        const auto &op = req.as<Type::kClose>();
        ::io_uring_prep_close(sqe, op.fd);
        break;
    }
    case IoRequest::Type::kFdatasync:
    case IoRequest::Type::kPoolAdd:
    case IoRequest::Type::kPoolRemove:
    case IoRequest::Type::kCancel:
        throw std::system_error(ENOSYS, std::system_category(), "io_uring: операция не реализована в prepare_sqe");
    }
    ::io_uring_sqe_set_data(sqe, completion);
}
} // namespace

void UringBackend::init(unsigned entries)
{
    const int ret = ::io_uring_queue_init(entries, &mRing, 0);
    if (ret < 0)
    {
        throw std::system_error(-ret, std::system_category(), "io_uring_queue_init");
    }
    mInitialized = true;
    mReady.reserve(entries);
}

void UringBackend::shutdown() noexcept
{
    if (mInitialized)
    {
        ::io_uring_queue_exit(&mRing);
        mInitialized = false;
        mInflight = 0;
        mHasPendingRequests = false;
        mReady.clear();
    }
}

UringBackend::~UringBackend()
{
    shutdown();
}

void UringBackend::flush()
{
    if (!mHasPendingRequests)
    {
        return;
    }

    for (;;)
    {
        const int ret = ::io_uring_submit(&mRing);
        if (ret >= 0)
        {
            mHasPendingRequests = false;
            return;
        }
        if (ret == -EINTR)
        {
            continue;
        }
        if (ret == -EBUSY)
        {
            harvestCompletions();
            continue;
        }
        throw std::system_error(-ret, std::system_category(), "io_uring_submit");
    }
}

::io_uring_sqe *UringBackend::tryGetSqe()
{
    return ::io_uring_get_sqe(&mRing);
}

::io_uring_sqe *UringBackend::getSqe()
{
    if (auto *sqe = tryGetSqe())
    {
        return sqe;
    }

    flush();
    if (auto *sqe = tryGetSqe())
    {
        return sqe;
    }

    harvestCompletions();
    flush();
    if (auto *sqe = tryGetSqe())
    {
        return sqe;
    }

    throw std::system_error(EBUSY, std::system_category(), "io_uring: SQ ring исчерпан, увеличьте queueDepth");
}

void UringBackend::harvestCompletions() noexcept
{
    ::io_uring_cqe *cqe = nullptr;
    unsigned head = 0;
    unsigned count = 0;

    io_uring_for_each_cqe(&mRing, head, cqe)
    {
        auto *completion = static_cast<IoCompletion *>(::io_uring_cqe_get_data(cqe));
        if (completion != nullptr)
        {
            mReady.push_back(Ready{completion, cqe->res});
        }
        ++count;
    }

    if (count > 0)
    {
        ::io_uring_cq_advance(&mRing, count);
        mInflight -= (count <= mInflight) ? count : mInflight;
    }
}

void UringBackend::dispatchReady()
{
    if (mDispatching)
    {
        return;
    }

    struct Guard
    {
        bool &flag;
        ~Guard()
        {
            flag = false;
        }
    } guard{mDispatching};
    mDispatching = true;

    for (std::size_t i = 0; i < mReady.size(); ++i)
    {
        const Ready ready = mReady[i];
        ready.completion->complete(ready.res);
    }
    mReady.clear();
}

void UringBackend::processCompletions()
{
    harvestCompletions();
    dispatchReady();
}

void UringBackend::submitRequest(const IoRequest &request, IoCompletion *completion)
{
    ::io_uring_sqe *sqe = getSqe();
    prepare_sqe(sqe, request, completion);
    mHasPendingRequests = true;
    ++mInflight;
}

void UringBackend::waitAndProcessCompletions()
{
    flush();

    if (mReady.empty())
    {
        ::io_uring_cqe *cqe = nullptr;
        const int ret = ::io_uring_wait_cqe(&mRing, &cqe);
        if (ret < 0)
        {
            if (ret == -EINTR || ret == -EAGAIN)
            {
                return;
            }
            throw std::system_error(-ret, std::system_category(), "io_uring_wait_cqe");
        }
    }

    processCompletions();
}
} // namespace pulsar::io_engine
