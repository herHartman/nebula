#include "test_support.hpp"

#include "pulsar/io_context.hpp"
#include "pulsar/net/tcp.hpp"

#include <gtest/gtest.h>

#include <string>
#include <system_error>

using namespace pulsar;
using namespace test_support;

namespace
{
Task<void> touchFlag(bool &flag)
{
    flag = true;
    co_return;
}

Task<void> bump(int &counter)
{
    ++counter;
    co_return;
}

Task<void> acceptForever(net::TcpAcceptor &acceptor)
{
    co_await acceptor.accept();
}

Task<void> stopContext(IoContext &ctx)
{
    ctx.stop();
    co_return;
}

Task<void> connectExpectingFailure(std::uint16_t port, std::error_code &out)
{
    net::TcpSocket socket;
    socket.open();
    try
    {
        co_await socket.connect(ip::Endpoint::v4("127.0.0.1", port));
    }
    catch (const std::system_error &e)
    {
        out = e.code();
    }
}

Task<void> spawnMany(int count, int &counter)
{
    for (int i = 0; i < count; ++i)
    {
        ioContext().spawn(bump(counter));
    }
    co_return;
}
} // namespace

TEST(IoContext, CurrentIsSetOnConstruction)
{
    SKIP_IF_NO_IO_URING();
    IoContext ctx;
    EXPECT_EQ(&IoContext::current(), &ctx);
    EXPECT_EQ(&ioContext(), &ctx);
}

TEST(IoContext, SpawnStartsTaskEagerly)
{
    SKIP_IF_NO_IO_URING();
    IoContext ctx;

    bool ran = false;
    ctx.spawn(touchFlag(ran));
    EXPECT_TRUE(ran);
}

TEST(IoContext, RunReturnsWhenAllTasksFinish)
{
    SKIP_IF_NO_IO_URING();
    IoContext ctx;

    int completed = 0;
    ctx.spawn(bump(completed));
    ctx.spawn(bump(completed));

    ctx.run();
    EXPECT_EQ(completed, 2);
}

TEST(IoContext, RunReturnsImmediatelyWithNoTasks)
{
    SKIP_IF_NO_IO_URING();
    IoContext ctx;
    ctx.run();
    SUCCEED();
}

TEST(IoContext, StopBreaksLoopWithPendingIo)
{
    SKIP_IF_NO_IO_URING();
    const auto port = freePort(SOCK_STREAM);

    IoContext ctx;
    net::TcpAcceptor acceptor;
    acceptor.open();
    acceptor.setReuseAddr();
    acceptor.bind(ip::Endpoint::any(port));
    acceptor.listen();

    ctx.spawn(acceptForever(acceptor)); // никогда не завершится сам
    ctx.spawn(stopContext(ctx));

    ctx.run();
    SUCCEED();
}

TEST(IoContext, ConnectToClosedPortThrowsConnectionRefused)
{
    SKIP_IF_NO_IO_URING();
    const auto port = freePort(SOCK_STREAM); // порт уже освобождён

    IoContext ctx;
    std::error_code captured;
    ctx.spawn(connectExpectingFailure(port, captured));
    ctx.run();

    EXPECT_EQ(captured.value(), ECONNREFUSED);
}

TEST(IoContext, NestedSpawnFromInsideCoroutine)
{
    SKIP_IF_NO_IO_URING();
    IoContext ctx;

    int inner = 0;
    ctx.spawn(spawnMany(64, inner));
    ctx.run();

    EXPECT_EQ(inner, 64);
}
