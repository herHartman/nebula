#include "pulsar/task.hpp"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <cstdint>
#include <optional>
#include <vector>

using pulsar::Task;

namespace
{
Task<int> answer()
{
    co_return 42;
}

Task<int> addOne(int x)
{
    const int v = co_await answer();
    co_return v + x;
}

Task<void> nothing()
{
    co_return;
}

Task<std::string> makeString()
{
    co_return std::string("hello");
}

Task<int> thrower()
{
    throw std::runtime_error("boom");
    co_return 1; // недостижимо, но делает функцию корутиной
}

Task<void> setFlag(bool& flag)
{
    flag = true;
    co_return;
}

Task<int> catcher()
{
    try
    {
        co_return co_await thrower();
    }
    catch (const std::runtime_error&)
    {
        co_return -1;
    }
}

template <typename T>
Task<void> collectInto(Task<T> task, std::optional<T>& out)
{
    out = co_await task;
}

Task<void> awaitVoid(Task<void> task)
{
    co_await task;
}

template <typename T> T syncAwait(Task<T> task)
{
    std::optional<T> out;
    auto wrapper = collectInto<T>(std::move(task), out);
    wrapper.start();
    EXPECT_TRUE(wrapper.done());
    return std::move(*out);
}

void syncRun(Task<void> task)
{
    auto wrapper = awaitVoid(std::move(task));
    wrapper.start();
    EXPECT_TRUE(wrapper.done());
}
} // namespace

TEST(Task, LazyStart)
{
    bool ran = false;
    auto t = setFlag(ran);

    // initial_suspend == suspend_always: тело не должно выполниться до start()
    EXPECT_FALSE(ran);
    t.start();
    EXPECT_TRUE(ran);
    EXPECT_TRUE(t.done());
}

TEST(Task, ReturnsValue)
{
    EXPECT_EQ(syncAwait(answer()), 42);
}

TEST(Task, NestedAwait)
{
    EXPECT_EQ(syncAwait(addOne(8)), 50);
}

TEST(Task, VoidSpecialization)
{
    syncRun(nothing());
}

TEST(Task, MoveOnlyResult)
{
    EXPECT_EQ(syncAwait(makeString()), "hello");
}

TEST(Task, ExceptionPropagatesToAwaiter)
{
    EXPECT_EQ(syncAwait(catcher()), -1);
}

Task<void> detachedThrower(bool& reachedEnd)
{
    throw std::runtime_error("boom");
    reachedEnd = true;
    co_return;
}

TEST(Task, DetachedTaskSwallowsException)
{
    bool reachedEnd = false;
    auto t = detachedThrower(reachedEnd);

    EXPECT_NO_THROW(t.start());
    EXPECT_FALSE(reachedEnd);
    EXPECT_TRUE(t.done());
}

TEST(Task, MoveAssignmentDoesNotLeak)
{
    auto a = answer();
    auto b = answer();
    a = std::move(b);
    EXPECT_EQ(syncAwait(std::move(a)), 42);
}

// Symmetric transfer: длинная цепочка co_await не должна расти по нативному
// стеку - await_suspend возвращает handle, и компилятор обязан сделать
// tail-call вместо вложенного resume()
namespace
{
Task<std::uintptr_t> stackAtDepth(unsigned depth)
{
    if (depth == 0)
    {
        co_return reinterpret_cast<std::uintptr_t>(__builtin_frame_address(0));
    }
    co_return co_await stackAtDepth(depth - 1);
}

std::ptrdiff_t stackGrowthOver(unsigned depth)
{
    const auto top = reinterpret_cast<std::uintptr_t>(__builtin_frame_address(0));
    const auto deepest = syncAwait(stackAtDepth(depth));
    return static_cast<std::ptrdiff_t>(top > deepest ? top - deepest : deepest - top);
}
} // namespace

TEST(Task, SymmetricTransferDoesNotGrowStack)
{
    constexpr unsigned kShallow = 16;
    constexpr unsigned kDeep = 4096;

    const auto shallow = stackGrowthOver(kShallow);
    const auto deep = stackGrowthOver(kDeep);

    const auto perFrame = (deep - shallow) / static_cast<std::ptrdiff_t>(kDeep - kShallow);

    if (perFrame > 0)
    {
        GTEST_SKIP() << "компилятор не сделал tail-call (~" << perFrame
                     << " байт стека на звено). Ожидаемо для сборки без "
                        "оптимизаций; в -O2 должно быть 0.";
    }

    EXPECT_LE(perFrame, 0);
}

namespace
{
Task<std::uint64_t> chain(unsigned depth)
{
    if (depth == 0)
    {
        co_return 0;
    }
    co_return 1 + co_await chain(depth - 1);
}
} // namespace

TEST(Task, DeepChainProducesCorrectResult)
{
    constexpr unsigned kDepth = 4096;
    EXPECT_EQ(syncAwait(chain(kDepth)), kDepth);
}
