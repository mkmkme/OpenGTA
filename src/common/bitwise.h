#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

consteval size_t getBitCount(size_t byteCount)
{
    return byteCount * 8;
}

namespace Util {

constexpr bool getBit(const std::unsigned_integral auto value, uint8_t bit)
{
    assert(bit >= 1 && bit <= getBitCount(sizeof(value)));
    return (value >> (bit - 1)) & 1;
}

constexpr void setBit(std::unsigned_integral auto *value, uint8_t bit, bool set)
{
    assert(bit >= 1 && bit <= getBitCount(sizeof(*value)));
    constexpr std::remove_pointer_t<decltype(value)> one = 1;
    const auto mask = one << (bit - 1);
    if (set)
        *value |= mask;
    else
        *value &= ~mask;
}

constexpr auto getRangeBit(const std::unsigned_integral auto value, uint8_t start, uint8_t end) -> decltype(value)
{
    constexpr auto bits = getBitCount(sizeof(value));
    assert(start >= 1 && start <= bits);
    assert(end >= start && end <= bits);
    const auto width = end - start + 1;
    const auto mask = std::numeric_limits<decltype(value)>::max() >> (bits - width);
    const auto val = value >> (start - 1);
    return static_cast<decltype(value)>(val & mask);
}

constexpr void setRangeBit(std::unsigned_integral auto *value, uint8_t start, uint8_t end, bool bit)
{
    using T = std::remove_pointer_t<decltype(value)>;
    constexpr auto bits = getBitCount(sizeof(T));
    assert(start >= 1 && start <= bits);
    assert(end >= start && end <= bits);
    const auto width = end - start + 1;
    const auto mask = std::numeric_limits<T>::max() >> (bits - width);
    if (bit)
        *value |= (mask << (start - 1));
    else
        *value &= ~(mask << (start - 1));
}

constexpr void copyRangeBit(
    std::unsigned_integral auto *value,
    uint8_t start,
    uint8_t end,
    std::unsigned_integral auto val
)
    requires(sizeof(val) <= sizeof(*value))
{
    using T = std::remove_pointer_t<decltype(value)>;
    constexpr auto bits = getBitCount(sizeof(T));
    assert(start >= 1 && start <= bits);
    assert(end >= start && end <= bits);
    const auto width = end - start + 1;
    assert(static_cast<T>(val) <= (std::numeric_limits<T>::max() >> (bits - width)));
    const auto mask = static_cast<T>(val) << (start - 1);
    setRangeBit(value, start, end, 0);
    *value |= mask;
}

} // namespace Util
