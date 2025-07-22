#pragma once

#include <array>
#include <cstdint>

namespace Util {
class PhysFSFile;
}

namespace OpenGTA {

struct DoorInfo {
    int16_t rpx, rpy;
    int16_t object;
    int16_t delta;
};

struct HlsInfo {
    int16_t h, l, s;
};

struct CarInfo {
    explicit CarInfo(Util::PhysFSFile &file);
    [[nodiscard]] uint32_t bytes_read() const noexcept { return bytes_read_; }
    int16_t width {}, height {}, depth {};
    int16_t sprNum {};
    int16_t weightDescriptor {};
    int16_t maxSpeed {}, minSpeed {};
    int16_t acceleration {}, braking {};
    int16_t grip {}, handling {};
    // ... remaps
    std::array<HlsInfo, 12> remap24 {};
    std::array<uint8_t, 12> remap8 {};
    uint8_t vtype {};
    uint8_t model {};
    uint8_t turning {};
    uint8_t damagable {};
    std::array<uint16_t, 4> value {};
    int8_t cx {}, cy {};
    uint32_t moment {};
    int16_t turnRatio {};
    int16_t driveWheelOffset {};
    int16_t steeringWheelOffset {};
    uint8_t convertible {};
    uint8_t engine {};
    uint8_t radio {};
    uint8_t horn {};
    uint8_t soundFunction {};
    uint8_t fastChangeFlag {};
    int16_t numDoors;
    std::array<DoorInfo, 4> door {}; // FIXME: MAX_DOORS

private:
    uint32_t bytes_read_ = 0;
};

} // namespace OpenGTA
