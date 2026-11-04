#pragma once

#include "pulsar/io/basic_socket.hpp"
#include "pulsar/result.hpp"
#include <cstddef>
#include <expected>
#include <span>
#include <system_error>

namespace pulsar::io
{
namespace detail
{
[[nodiscard]] inline std::error_code error_from_res(int res) noexcept
{
    return {-res, std::system_category()};
}
} // namespace detail

template <typename Protocol>
class BasicStreamSocket : public BasicSocket<Protocol>
{
  public:
    using BasicSocket<Protocol>::BasicSocket;

    Task<Result<std::size_t>> read(std::span<std::byte> buffer)
    {
        const auto recvRequest = IoRequest::makeRecv(this->fd(), buffer.size(), buffer.data());
        const int n = co_await IoAwaitable{recvRequest};

        if (n < 0)
        {
            co_return std::unexpected(detail::error_from_res(n));
        }
        co_return static_cast<std::size_t>(n);
    }

    Task<Result<std::size_t>> write(std::span<const std::byte> buffer)
    {
        std::byte *rawBuffer = const_cast<std::byte *>(buffer.data());
        const auto sendRequest = IoRequest::makeSend(this->fd(), buffer.size(), rawBuffer, SendFlags::kMsgNosignal);
        const int n = co_await IoAwaitable{sendRequest};

        if (n < 0)
        {
            co_return std::unexpected(detail::error_from_res(n));
        }
        co_return static_cast<std::size_t>(n);
    }

    Task<Result<void>> writeAll(std::span<const std::byte> buffer)
    {
        while (!buffer.empty())
        {
            const auto result = co_await write(buffer);
            if (!result)
            {
                co_return std::unexpected(result.error());
            }
            if (result.value() == 0)
            {
                co_return std::unexpected(std::make_error_code(std::errc::io_error));
            }
            buffer = buffer.subspan(result.value());
        }
    }
};
} // namespace pulsar::io
