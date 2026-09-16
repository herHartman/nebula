#pragma once

#include "io_completion.h"
#include "io_operation.h"
#include "task.hpp"
#include "uring_backend.hpp"
#include <coroutine>
#include <exception>
#include <functional>
#include <system_error>
#include <vector>

namespace pulsar
{
class IoContext
{
  public:
    explicit IoContext(unsigned queueDepth = 256);
    ~IoContext();

    IoContext(const IoContext &) = delete;
    IoContext &operator=(const IoContext &) = delete;

    void run();

    void stop() noexcept;

    void restart() noexcept;

    void submit(const IoRequest &request, IoCompletion *completion);

    void spawn(Task<void> task);

    void setExceptionHandler(std::function<void(std::exception_ptr)> handler);

    static IoContext &current();

  private:
    void reapFinishedTasks();
    void reportTaskException(std::exception_ptr e) const noexcept;

    io_engine::UringBackend mBackend;
    std::function<void(std::exception_ptr)> mExceptionHandler;
    bool mStopped = false;
    std::vector<Task<void>> mBackgroundTasks;
};

inline IoContext &ioContext()
{
    return IoContext::current();
}

class IoAwaitable
{
  public:
    explicit IoAwaitable(IoRequest request) noexcept : mRequest(request)
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<> continuation)
    {
        mCompletion.setContinuation(continuation);
        IoContext::current().submit(mRequest, &mCompletion);
    }

    int await_resume() const
    {
        int result = mCompletion.result();
        if (result < 0)
        {
            throw std::system_error(-result, std::system_category());
        }
        return result;
    }

  private:
    IoRequest mRequest;
    IoCompletion mCompletion;
};

void runOnEachCore(unsigned coreCount, const std::function<Task<void>(unsigned)> &main);
} // namespace pulsar
