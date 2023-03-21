#pragma once

#include <cassert>
#include <concepts>
#include <cstdint>

namespace {

consteval size_t getBitCount(size_t byteCount)
{
    return byteCount * 8;
}

} // namespace

namespace Util {

inline constexpr bool getBit(const std::unsigned_integral auto value, uint8_t bit)
{
    assert(bit >= 1 && bit <= getBitCount(sizeof(value)));
    return (value >> (bit - 1)) & 1;
}

inline constexpr void setBit(std::unsigned_integral auto *value, uint8_t bit, bool set)
{
    assert(bit >= 1 && bit <= getBitCount(sizeof(value)));
    if (set)
        *value |= (1 << (bit - 1));
    else
        *value &= ~(1 << (bit - 1));
}

constexpr auto getRangeBit(const std::unsigned_integral auto value, uint8_t start, uint8_t end) -> decltype(value)
{
    assert(start >= 1 && start <= getBitCount(sizeof(value)));
    assert(end >= start && end <= getBitCount(sizeof(value)));
    constexpr decltype(value) one = 1;
    const auto mask = (one << (end - start + 1)) - one;
    const auto val = value >> (start - 1);
    return static_cast<decltype(value)>(val & mask);
}

constexpr void setRangeBit(std::unsigned_integral auto *value, uint8_t start, uint8_t end, bool bit)
{
    assert(start >= 1 && start <= getBitCount(sizeof(value)));
    assert(end >= start && end <= getBitCount(sizeof(value)));
    constexpr std::remove_pointer_t<decltype(value)> one = 1;
    const auto mask = (one << (end - start + 1)) - one;
    if (bit)
        *value |= (mask << (start - 1));
    else
        *value &= ~(mask << (start - 1));
}

constexpr void copyRangeBit(
    std::unsigned_integral auto *value, uint8_t start, uint8_t end, std::unsigned_integral auto val
)
    requires (sizeof(val) <= sizeof(*value))
{
    const auto mask = val << (start - 1);
    assert(start >= 1 && start <= getBitCount(sizeof(value)));
    assert(end >= start && end <= getBitCount(sizeof(value)));
    setRangeBit(value, start, end, 0);
    *value |= mask;
}

} // namespace Util
