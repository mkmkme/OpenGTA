#pragma once

#include <type_traits>

#include <physfs.h>

using Int8 = PHYSFS_sint8;
using UInt8 = PHYSFS_uint8;
using Int16 = PHYSFS_sint16;
using UInt16 = PHYSFS_uint16;
using UInt32 = PHYSFS_uint32;
using UInt64 = PHYSFS_uint64;

template <typename T>
concept BuiltinNumber =
    std::is_same_v<T, Int8> ||
    std::is_same_v<T, UInt8> ||
    std::is_same_v<T, Int16> ||
    std::is_same_v<T, UInt16> ||
    std::is_same_v<T, UInt32> ||
    std::is_same_v<T, UInt64>;
