#include "pulsar/io_context.hpp"
#include "pulsar/net/tcp.hpp"
#include <array>
#include <cstddef>
#include <iostream>

using namespace pulsar;

namespace
{
Task<void> handleConnection(net::TcpSocket socket)
{
    std::array<std::byte, 4096> buffer;
    try
    {
        while (true)
        {
            std::size_t n = co_await socket.read(buffer);
            if (n == 0)
            {
                break;
            }
            co_await socket.writeAll(std::span(buffer).first(n));
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "connection error: " << e.what() << '\n';
    }
    co_await socket.close();
}

Task<void> acceptLoop(net::TcpAcceptor& acceptor)
{
    while (true)
    {
        net::TcpSocket socket = co_await acceptor.accept();
        ioContext().spawn(handleConnection(std::move(socket)));
    }
}
} // namespace

int main()
{
    IoContext ctx;

    net::TcpAcceptor acceptor;
    acceptor.open();
    acceptor.setReuseAddr();
    acceptor.bind(ip::Endpoint::any(9000));
    acceptor.listen();

    std::cout << "tcp echo server listening on 0.0.0.0:9000\n";

    ctx.spawn(acceptLoop(acceptor));
    ctx.run();
}
