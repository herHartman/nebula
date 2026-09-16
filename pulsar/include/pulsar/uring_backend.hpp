#pragma once

#include "io_completion.h"
#include "io_operation.h"
#include <cstddef>
#include <liburing.h>
#include <vector>

namespace pulsar::io_engine
{
class UringBackend
{
  public:
    UringBackend() = default;
    ~UringBackend();

    UringBackend(const UringBackend &) = delete;
    UringBackend &operator=(const UringBackend &) = delete;
    UringBackend(UringBackend &&) = delete;
    UringBackend &operator=(UringBackend &&) = delete;

    void init(unsigned entries = 256);

    void shutdown() noexcept;

    void submitRequest(const IoRequest &request, IoCompletion *completion);
    void waitAndProcessCompletions();

    bool hasInflight() const noexcept
    {
        return mInflight > 0;
    }

  private:
    struct Ready
    {
        IoCompletion *completion;
        int res;
    };

    void harvestCompletions() noexcept;

    void dispatchReady();

    void processCompletions();

    void flush();

    ::io_uring_sqe *tryGetSqe();

    ::io_uring_sqe *getSqe();

    ::io_uring mRing{};
    std::vector<Ready> mReady;
    std::size_t mInflight = 0;
    bool mHasPendingRequests = false;
    bool mInitialized = false;
    bool mDispatching = false;
};
} // namespace pulsar::io_engine
