#pragma once

#include <type_traits>

namespace pulsar
{
template <typename T>
concept EnumClass = std::is_enum_v<T>;

template <EnumClass Enum>
class Flags
{
    using underlying = std::underlying_type_t<Enum>;
    underlying mValue;

  public:
    constexpr Flags() noexcept : mValue(0)
    {
    }

    constexpr explicit Flags(Enum e) noexcept : mValue(static_cast<underlying>(e))
    {
    }

    constexpr Flags operator|(Enum e) const noexcept
    {
        return Flags{mValue | static_cast<underlying>(e)};
    }

    constexpr Flags operator|(Flags other) const noexcept
    {
        return Flags{mValue | other.mValue};
    }

    constexpr Flags &operator|=(Enum e) noexcept
    {
        mValue |= static_cast<underlying>(e);
        return *this;
    }

    constexpr bool has(Enum e) const noexcept
    {
        return (mValue & static_cast<underlying>(e)) != 0;
    }

    [[nodiscard]] constexpr underlying get() const noexcept
    {
        return mValue;
    }
    constexpr explicit operator bool() const noexcept
    {
        return mValue != 0;
    }

    constexpr bool operator==(const Flags &other) const noexcept = default;

  private:
    constexpr explicit Flags(underlying v) noexcept : mValue(v)
    {
    }
};
} // namespace pulsar
