#pragma once

#include <physfs.h>

namespace OpenGTA {

struct DoorInfo {
    PHYSFS_sint16 rpx, rpy;
    PHYSFS_sint16 object;
    PHYSFS_sint16 delta;
};

struct HlsInfo {
    PHYSFS_sint16 h, l, s;
};

struct CarInfo {
    PHYSFS_sint16 width, height, depth;
    PHYSFS_sint16 sprNum;
    PHYSFS_sint16 weightDescriptor;
    PHYSFS_sint16 maxSpeed, minSpeed;
    PHYSFS_sint16 acceleration, braking;
    PHYSFS_sint16 grip, handling;
    // ... remaps
    HlsInfo remap24[12];
    PHYSFS_uint8 remap8[12];
    PHYSFS_uint8 vtype;
    PHYSFS_uint8 model;
    PHYSFS_uint8 turning;
    PHYSFS_uint8 damagable;
    PHYSFS_uint16 value[4];
    PHYSFS_sint8 cx, cy;
    PHYSFS_uint32 moment;
    float rbpMass;
    float g1_Thrust;
    float tyreAdhesionX, tyreAdhesionY;
    float handBrakeFriction;
    float footBrakeFriction;
    float frontBrakeBias;
    PHYSFS_sint16 turnRatio;
    PHYSFS_sint16 driveWheelOffset;
    PHYSFS_sint16 steeringWheelOffset;
    float backEndSlideValue;
    float handBrakeSlideValue;
    PHYSFS_uint8 convertible;
    PHYSFS_uint8 engine;
    PHYSFS_uint8 radio;
    PHYSFS_uint8 horn;
    PHYSFS_uint8 soundFunction;
    PHYSFS_uint8 fastChangeFlag;
    PHYSFS_sint16 numDoors;
    DoorInfo door[4]; // FIXME: MAX_DOORS
};

} // namespace OpenGTA
