#pragma once

#include "pulsar/result.hpp"
#include "pulsar/task.hpp"

#include <algorithm>
#include <system_error>
#include <variant>
#include <vector>
#include <string_view>
#include <span>

namespace nebula::http::test
{
class FakeStream
{
public:
    using Step = std::variant<std::vector<std::byte>, std::error_code>;

    FakeStream& appendData(std::string_view data)
    {
        auto bytes = std::as_bytes(std::span{data});
        mSteps.emplace_back(std::vector<std::byte>(bytes.begin(), bytes.end()));
        return *this;
    }

    FakeStream& appendError(std::error_code ec)
    {
        mSteps.emplace_back(std::move(ec));
        return *this;
    }

    pulsar::Task<pulsar::Result<std::size_t>> read(std::span<std::byte> buffer)
    {
        if (mStep == mSteps.size())
        {
            co_return 0;
        }

        if (const std::error_code* ec = std::get_if<std::error_code>(&mSteps[mStep]))
        {
            co_return std::unexpected(*ec);
        }

        std::vector<std::byte>& chunk = std::get<std::vector<std::byte>>(mSteps[mStep]);
        const std::size_t toCopy = std::min(buffer.size(), chunk.size());
        std::ranges::copy(std::span(chunk).subspan(mOffset, toCopy), buffer.begin());
        mOffset += toCopy;

        if (mOffset == chunk.size()) 
        {
            ++mStep;
        }

        co_return toCopy;
    }

private:
    std::vector<Step> mSteps;
    std::size_t mOffset = 0;
    std::size_t mStep = 0;
};
} // namespace nebula::http::test
