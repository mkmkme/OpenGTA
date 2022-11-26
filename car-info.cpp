#include "car-info.h"

#include "util/log.h"

OpenGTA::CarInfo::CarInfo(PHYSFS_file *fd)
{
    PHYSFS_readSLE16(fd, &width);
    PHYSFS_readSLE16(fd, &height);
    PHYSFS_readSLE16(fd, &depth);
    PHYSFS_readSLE16(fd, &sprNum);
    PHYSFS_readSLE16(fd, &weightDescriptor);
    PHYSFS_readSLE16(fd, &maxSpeed);
    PHYSFS_readSLE16(fd, &minSpeed);
    PHYSFS_readSLE16(fd, &acceleration);
    PHYSFS_readSLE16(fd, &braking);
    PHYSFS_readSLE16(fd, &grip);
    PHYSFS_readSLE16(fd, &handling);
    bytes_read_ += 2 * 11;

    for (int i = 0; i < 12; i++) {
        PHYSFS_readSLE16(fd, &remap24[i].h);
        PHYSFS_readSLE16(fd, &remap24[i].l);
        PHYSFS_readSLE16(fd, &remap24[i].s);
    }
    bytes_read_ += 12 * 3 * 2;
    for (int i = 0; i < 12; i++)
        PHYSFS_readBytes(fd, static_cast<void *>(&remap8[i]), 1);
    bytes_read_ += 12;

    PHYSFS_readBytes(fd, static_cast<void *>(&vtype), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&model), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&turning), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&damagable), 1);
    bytes_read_ += 4;

    for (int i = 0; i < 4; i++)
        PHYSFS_readULE16(fd, &value[i]);
    bytes_read_ += 4 * 2;

    PHYSFS_readBytes(fd, static_cast<void *>(&cx), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&cy), 1);
    PHYSFS_readULE32(fd, &moment);
    bytes_read_ += 2 + 4;

    PHYSFS_uint32 fixed_tmp;
    PHYSFS_readULE32(fd, &fixed_tmp);
    rbpMass = float(fixed_tmp) / 65536;

    PHYSFS_readULE32(fd, &fixed_tmp);
    g1_Thrust = float(fixed_tmp) / 65536;

    PHYSFS_readULE32(fd, &fixed_tmp);
    tyreAdhesionX = float(fixed_tmp) / 65536;

    PHYSFS_readULE32(fd, &fixed_tmp);
    tyreAdhesionY = float(fixed_tmp) / 65536;

    PHYSFS_readULE32(fd, &fixed_tmp);
    handBrakeFriction = float(fixed_tmp) / 65536;

    PHYSFS_readULE32(fd, &fixed_tmp);
    footBrakeFriction = float(fixed_tmp) / 65536;

    PHYSFS_readULE32(fd, &fixed_tmp);
    frontBrakeBias = float(fixed_tmp) / 65536;

    bytes_read_ += 7 * 4;

    PHYSFS_readSLE16(fd, &turnRatio);
    PHYSFS_readSLE16(fd, &driveWheelOffset);
    PHYSFS_readSLE16(fd, &steeringWheelOffset);
    bytes_read_ += 3 * 2;

    PHYSFS_readULE32(fd, &fixed_tmp);
    backEndSlideValue = float(fixed_tmp) / 65536;

    PHYSFS_readULE32(fd, &fixed_tmp);
    handBrakeSlideValue = float(fixed_tmp) / 65536;

    bytes_read_ += 2 * 4;

    PHYSFS_readBytes(fd, static_cast<void *>(&convertible), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&engine), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&radio), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&horn), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&soundFunction), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&fastChangeFlag), 1);
    bytes_read_ += 6;

    PHYSFS_readSLE16(fd, &numDoors);
    bytes_read_ += 2;
    if (numDoors > 2) {
        WARN("num-doors: {} > 2 ???", numDoors);
        numDoors = 0;
    }

    for (int i = 0; i < numDoors; i++) {
        PHYSFS_readSLE16(fd, &door[i].rpy);
        PHYSFS_readSLE16(fd, &door[i].rpx);
        PHYSFS_readSLE16(fd, &door[i].object);
        PHYSFS_readSLE16(fd, &door[i].delta);
        bytes_read_ += 4 * 2;
    }

    DEBUG("car of size {} is read ({} doors)", int(bytes_read_), int(numDoors));
}
