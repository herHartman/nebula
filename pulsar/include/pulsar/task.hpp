#pragma once

#include <coroutine>
#include <exception>
#include <optional>
#include <type_traits>
#include <utility>

namespace pulsar
{
namespace detail
{
template <typename T>
struct PromiseValue
{
    std::optional<T> value;

    template <typename U>
    void return_value(U &&v)
    {
        value.emplace(std::forward<U>(v));
    }

    T &result()
    {
        return *value;
    }
};

template <>
struct PromiseValue<void>
{
    void return_void() noexcept
    {
    }

    void result() noexcept
    {
    }
};
} // namespace detail

template <typename T = void>
class [[nodiscard]] Task
{
  public:
    struct promise_type;
    using Handle = std::coroutine_handle<promise_type>;

    struct promise_type : detail::PromiseValue<T>
    {
        std::coroutine_handle<> continuation;
        std::exception_ptr exception;

        Task get_return_object() noexcept
        {
            return Task{Handle::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept
        {
            return {};
        }

        struct FinalAwaiter
        {
            bool await_ready() noexcept
            {
                return false;
            }

            std::coroutine_handle<> await_suspend(Handle h) noexcept
            {
                auto oldContinuation = h.promise().continuation;
                return oldContinuation ? oldContinuation : std::noop_coroutine();
            }

            void await_resume() noexcept
            {
            }
        };

        FinalAwaiter final_suspend() noexcept
        {
            return {};
        }

        void unhandled_exception() noexcept
        {
            exception = std::current_exception();
        }
    };

    Task() noexcept = default;
    explicit Task(Handle h) noexcept : mHandle(h)
    {
    }

    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    Task(Task &&other) noexcept : mHandle(std::exchange(other.mHandle, {}))
    {
    }

    Task &operator=(Task &&other) noexcept
    {
        if (this != &other)
        {
            destroy();
            mHandle = std::exchange(other.mHandle, {});
        }
        return *this;
    }

    ~Task()
    {
        destroy();
    }

    bool done() const noexcept
    {
        return !mHandle || mHandle.done();
    }

    void start()
    {
        mHandle.resume();
    }

    std::exception_ptr exception() const noexcept
    {
        return mHandle ? mHandle.promise().exception : nullptr;
    }

    struct Awaiter
    {
        Handle mHandle;

        bool await_ready() noexcept
        {
            return false;
        }

        Handle await_suspend(std::coroutine_handle<> awaiting) noexcept
        {
            mHandle.promise().continuation = awaiting;
            return mHandle;
        }

        decltype(auto) await_resume()
        {
            auto &promise = mHandle.promise();
            if (promise.exception)
            {
                std::rethrow_exception(promise.exception);
            }
            if constexpr (!std::is_void_v<T>)
            {
                return std::move(promise.result());
            }
            else
            {
                promise.result();
            }
        }
    };

    Awaiter operator co_await() const & noexcept
    {
        return Awaiter{mHandle};
    }

  private:
    void destroy()
    {
        if (mHandle)
        {
            mHandle.destroy();
        }
    }

    Handle mHandle;
};
} // namespace pulsar
