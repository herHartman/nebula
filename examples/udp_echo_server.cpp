#include "pulsar/io_context.hpp"
#include "pulsar/net/udp.hpp"
#include <array>
#include <iostream>

using namespace pulsar;

namespace
{
Task<void> echoLoop(net::UdpSocket &socket)
{
    std::array<std::byte, 4096> buffer;
    while (true)
    {
        auto received = co_await socket.receiveFrom(buffer);
        co_await socket.sendTo(std::span(buffer).first(received.size), received.from);
    }
}
} // namespace

int main()
{
    IoContext ctx;

    net::UdpSocket socket;
    socket.open();
    socket.setReuseAddr();
    socket.bind(ip::Endpoint::any(9001));

    std::cout << "udp echo server listening on 0.0.0.0:9001\n";

    ctx.spawn(echoLoop(socket));
    ctx.run();
}
