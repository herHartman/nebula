#include "nebula/http/detail/utils.hpp"
#include <gtest/gtest.h>

namespace nebula::http::detail::test
{
TEST(PercentDecode, DecodeSimpleUrl)
{
    constexpr std::string_view kNormalUrl = "http://test.com";
    std::string out;

    EXPECT_TRUE(percent_decode(kNormalUrl, out));
    EXPECT_EQ(kNormalUrl, out);
}

TEST(PercentDecode, DecodeUrlEncoded)
{
    constexpr std::string_view kUrl = "https://test.com/hello%20world%3F";
    std::string out;

    EXPECT_TRUE(percent_decode(kUrl, out));
    EXPECT_EQ(out, "https://test.com/hello world?");
}

TEST(PercentDecode, IncorrectString)
{
    std::string out;
    EXPECT_FALSE(percent_decode("https://test.com/hello%2world%3F", out));
    out.clear();

    EXPECT_FALSE(percent_decode("http://test.com/hello%test", out));
    out.clear();

    EXPECT_FALSE(percent_decode("http://test.com/hello%", out));
    out.clear();

    EXPECT_FALSE(percent_decode("http://test.com/hello%3X", out));
    out.clear();
}

TEST(PercentDecode, EmptyString)
{
    std::string out;

    EXPECT_TRUE(percent_decode("", out));
    EXPECT_TRUE(out.empty());
}

struct Case
{
    std::string_view input;
    std::optional<std::string_view> expected;
};

TEST(PercentDecode, Table)
{
    const Case cases[] = {
        {"abc%20", "abc "}, {"abc%2", std::nullopt}, {"%2520", "%20"},
        {"%2f", "/"},       {"%-1", std::nullopt},   {"%FF", "\xFF"},
    };

    for (const auto &[input, expected] : cases)
    {
        SCOPED_TRACE(input);
        std::string out = "garbage";
        const bool ok = percent_decode(input, out);
        ASSERT_EQ(ok, expected.has_value());
        if (expected)
        {
            EXPECT_EQ(out, *expected);
        }
    }
}
} // namespace nebula::http::detail::test
