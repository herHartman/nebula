#include "test_support.hpp"

#include "pulsar/io_context.hpp"
#include "pulsar/net/tcp.hpp"
#include "pulsar/net/udp.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <vector>

using namespace pulsar;
using namespace test_support;

namespace
{
Task<std::string> readExactly(net::TcpSocket& socket, std::size_t expected)
{
    std::string out;
    out.reserve(expected);
    std::vector<std::byte> buffer(64u * 1024u);

    while (out.size() < expected)
    {
        const std::size_t n = co_await socket.read(buffer);
        if (n == 0)
        {
            break;
        }
        out.append(reinterpret_cast<const char*>(buffer.data()), n);
    }
    co_return out;
}

Task<void> echoOnce(net::TcpAcceptor& acceptor)
{
    net::TcpSocket socket = co_await acceptor.accept();
    std::vector<std::byte> buffer(64u * 1024u);
    while (true)
    {
        const std::size_t n = co_await socket.read(buffer);
        if (n == 0)
        {
            break;
        }
        co_await socket.writeAll(std::span(buffer).first(n));
    }
    co_await socket.close();
}

Task<void> tcpClientOnce(std::uint16_t port, std::string message, std::string& out)
{
    net::TcpSocket socket;
    socket.open();
    co_await socket.connect(ip::Endpoint::v4("127.0.0.1", port));
    co_await socket.writeAll(asBytes(message));
    out = co_await readExactly(socket, message.size());
    co_await socket.close();
}

Task<void> writeAllInBackground(net::TcpSocket& socket, const std::string& payload)
{
    co_await socket.writeAll(asBytes(payload));
}

Task<void> tcpLargeClient(std::uint16_t port, const std::string& payload, std::string& out)
{
    net::TcpSocket socket;
    socket.open();
    co_await socket.connect(ip::Endpoint::v4("127.0.0.1", port));

    ioContext().spawn(writeAllInBackground(socket, payload));
    out = co_await readExactly(socket, payload.size());

    co_await socket.close();
}

Task<void> handleShortRequest(net::TcpSocket socket, int& served)
{
    std::array<std::byte, 256> buffer{};
    const std::size_t n = co_await socket.read(buffer);
    if (n > 0)
    {
        co_await socket.writeAll(std::span(buffer).first(n));
    }
    ++served;
    co_await socket.close();
}

Task<void> acceptLoop(net::TcpAcceptor& acceptor, int count, int& served)
{
    for (int i = 0; i < count; ++i)
    {
        net::TcpSocket socket = co_await acceptor.accept();
        ioContext().spawn(handleShortRequest(std::move(socket), served));
    }
}

Task<void> shortRoundTrip(std::uint16_t port, std::string message, int& ok)
{
    net::TcpSocket socket;
    socket.open();
    co_await socket.connect(ip::Endpoint::v4("127.0.0.1", port));
    co_await socket.writeAll(asBytes(message));

    std::array<std::byte, 256> buffer{};
    const std::size_t n = co_await socket.read(buffer);
    if (toString(std::span(buffer).first(n)) == message)
    {
        ++ok;
    }
    co_await socket.close();
}

Task<void> udpEchoOnce(net::UdpSocket& server)
{
    std::array<std::byte, 2048> buffer{};
    auto received = co_await server.receiveFrom(buffer);
    co_await server.sendTo(std::span(buffer).first(received.size), received.from);
}

Task<void> udpClient(std::uint16_t serverPort, std::string& echoed, std::string& peer)
{
    net::UdpSocket client;
    client.open();

    co_await client.sendTo(asBytes("udp-ping"), ip::Endpoint::v4("127.0.0.1", serverPort));

    std::array<std::byte, 2048> buffer{};
    auto received = co_await client.receiveFrom(buffer);
    echoed = toString(std::span(buffer).first(received.size));
    peer = received.from.address();
}

net::TcpAcceptor makeAcceptor(std::uint16_t port)
{
    net::TcpAcceptor acceptor;
    acceptor.open();
    acceptor.setReuseAddr();
    acceptor.bind(ip::Endpoint::any(port));
    acceptor.listen();
    return acceptor;
}
} // namespace

TEST(Net, TcpEchoRoundTrip)
{
    SKIP_IF_NO_IO_URING();
    const auto port = freePort(SOCK_STREAM);

    IoContext ctx;
    auto acceptor = makeAcceptor(port);

    std::string received;
    ctx.spawn(echoOnce(acceptor));
    ctx.spawn(tcpClientOnce(port, "hello io_uring", received));

    ctx.run();
    EXPECT_EQ(received, "hello io_uring");
}

TEST(Net, TcpLargePayloadPartialWrites)
{
    SKIP_IF_NO_IO_URING();
    const auto port = freePort(SOCK_STREAM);

    std::string payload(4u * 1024u * 1024u, '\0');
    for (std::size_t i = 0; i < payload.size(); ++i)
    {
        payload[i] = static_cast<char>('a' + (i % 26u));
    }

    IoContext ctx;
    auto acceptor = makeAcceptor(port);

    std::string received;
    ctx.spawn(echoOnce(acceptor));
    ctx.spawn(tcpLargeClient(port, payload, received));

    ctx.run();
    ASSERT_EQ(received.size(), payload.size());
    EXPECT_EQ(received, payload);
}

TEST(Net, ManyConnectionsWithTinyQueueDepth)
{
    SKIP_IF_NO_IO_URING();
    constexpr int kConnections = 64;
    const auto port = freePort(SOCK_STREAM);

    IoContext ctx{8};
    auto acceptor = makeAcceptor(port);

    int served = 0;
    int ok = 0;
    ctx.spawn(acceptLoop(acceptor, kConnections, served));

    for (int i = 0; i < kConnections; ++i)
    {
        ctx.spawn(shortRoundTrip(port, "conn-" + std::to_string(i), ok));
    }

    ctx.run();
    EXPECT_EQ(ok, kConnections);
    EXPECT_EQ(served, kConnections);
}

TEST(Net, UdpEchoRoundTrip)
{
    SKIP_IF_NO_IO_URING();
    const auto serverPort = freePort(SOCK_DGRAM);

    IoContext ctx;

    net::UdpSocket server;
    server.open();
    server.setReuseAddr();
    server.bind(ip::Endpoint::any(serverPort));

    std::string echoed;
    std::string peerAddress;
    ctx.spawn(udpEchoOnce(server));
    ctx.spawn(udpClient(serverPort, echoed, peerAddress));

    ctx.run();
    EXPECT_EQ(echoed, "udp-ping");
    EXPECT_EQ(peerAddress, "127.0.0.1");
}

TEST(Net, SocketIsMoveOnly)
{
    SKIP_IF_NO_IO_URING();
    net::TcpSocket a;
    a.open();
    const int fd = a.fd();
    ASSERT_GE(fd, 0);

    net::TcpSocket b = std::move(a);
    EXPECT_EQ(b.fd(), fd);
    EXPECT_FALSE(a.isOpen());
    EXPECT_TRUE(b.isOpen());
}
