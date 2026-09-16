#pragma once

#include "pulsar/io/datagram_socket.hpp"
#include "pulsar/net/protocol.hpp"

namespace pulsar::net
{
using UdpSocket = io::BasicDatagramSocket<ip::udp>;
} // namespace pulsar::net
