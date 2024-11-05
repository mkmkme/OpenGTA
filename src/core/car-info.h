#pragma once

#include <array>

#include <core/numeric-types.h>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

struct DoorInfo {
    Int16 rpx, rpy;
    Int16 object;
    Int16 delta;
};

struct HlsInfo {
    Int16 h, l, s;
};

struct CarInfo {
    explicit CarInfo(Util::PhysFSFile &file);
    [[nodiscard]] inline UInt32 bytes_read() const noexcept { return bytes_read_; }
    Int16 width {}, height {}, depth {};
    Int16 sprNum {};
    Int16 weightDescriptor {};
    Int16 maxSpeed {}, minSpeed {};
    Int16 acceleration {}, braking {};
    Int16 grip {}, handling {};
    // ... remaps
    std::array<HlsInfo, 12> remap24 {};
    std::array<UInt8, 12> remap8 {};
    UInt8 vtype {};
    UInt8 model {};
    UInt8 turning {};
    UInt8 damagable {};
    std::array<UInt16, 4> value {};
    Int8 cx {}, cy {};
    UInt32 moment {};
    Int16 turnRatio {};
    Int16 driveWheelOffset {};
    Int16 steeringWheelOffset {};
    UInt8 convertible {};
    UInt8 engine {};
    UInt8 radio {};
    UInt8 horn {};
    UInt8 soundFunction {};
    UInt8 fastChangeFlag {};
    Int16 numDoors;
    std::array<DoorInfo, 4> door {}; // FIXME: MAX_DOORS

private:
    UInt32 bytes_read_ = 0;
};

} // namespace OpenGTA
