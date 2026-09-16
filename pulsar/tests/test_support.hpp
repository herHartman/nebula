#pragma once

#include <gtest/gtest.h>
#include <liburing.h>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace test_support
{
inline bool ioUringAvailable()
{
    static const bool available = []
    {
        ::io_uring ring{};
        const int ret = ::io_uring_queue_init(8, &ring, 0);
        if (ret < 0)
        {
            return false;
        }
        ::io_uring_queue_exit(&ring);
        return true;
    }();
    return available;
}

#define SKIP_IF_NO_IO_URING()                                                                      \
    do                                                                                             \
    {                                                                                              \
        if (!::test_support::ioUringAvailable())                                                   \
        {                                                                                          \
            GTEST_SKIP() << "io_uring недоступен в этом окружении";                                \
        }                                                                                          \
    } while (false)

inline std::span<const std::byte> asBytes(std::string_view s) noexcept
{
    return {reinterpret_cast<const std::byte*>(s.data()), s.size()};
}

inline std::string toString(std::span<const std::byte> b)
{
    return {reinterpret_cast<const char*>(b.data()), b.size()};
}

std::uint16_t freePort(int sockType);
} // namespace test_support
