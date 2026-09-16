#pragma once

#include <coroutine>
#include <utility>

namespace pulsar
{
class IoCompletion
{
  public:
    void complete(int result) noexcept
    {
        mResult = result;
        if (mContinuation)
        {
            std::exchange(mContinuation, nullptr).resume();
        }
    }

    void setContinuation(std::coroutine_handle<> continuation) noexcept
    {
        mContinuation = continuation;
    }

    // cqe->res как есть: >=0 - байты/fd/etc, <0 - -errno
    int result() const noexcept
    {
        return mResult;
    }

  private:
    std::coroutine_handle<> mContinuation;
    int mResult = 0;
};
} // namespace pulsar
