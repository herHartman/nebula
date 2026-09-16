#include "pulsar/io_context.hpp"
#include "pulsar/net/tcp.hpp"
#include <array>
#include <iostream>
#include <string>
#include <string_view>

using namespace pulsar;

namespace
{
Task<void> run(IoContext& ctx, std::string message)
{
    net::TcpSocket socket;
    socket.open();
    co_await socket.connect(ip::Endpoint::v4("127.0.0.1", 9000));

    std::span<const std::byte> payload{
        reinterpret_cast<const std::byte*>(message.data()), message.size()};
    co_await socket.writeAll(payload);

    std::array<std::byte, 4096> buffer;
    std::size_t n = co_await socket.read(buffer);
    std::cout << "echo: "
              << std::string_view(reinterpret_cast<const char*>(buffer.data()), n) << '\n';

    co_await socket.close();
    ctx.stop();
}
} // namespace

int main(int argc, char** argv)
{
    std::string message = argc > 1 ? argv[1] : "hello io_uring";

    IoContext ctx;
    ctx.spawn(run(ctx, message));
    ctx.run();
}
