#pragma once

#include "pulsar/io/acceptor.hpp"
#include "pulsar/io/basic_stream_socket.hpp"
#include "pulsar/net/protocol.hpp"

namespace pulsar::net
{
using TcpSocket = io::BasicStreamSocket<ip::tcp>;
using TcpAcceptor = io::BasicAcceptor<ip::tcp>;
} // namespace pulsar::net
