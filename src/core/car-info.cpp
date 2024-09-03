#include <core/car-info.h>

#include <util/file-manager.h>
#include <util/log.h>

OpenGTA::CarInfo::CarInfo(Util::PhysFSFile &file)
{
    file.read(width);
    file.read(height);
    file.read(depth);
    file.read(sprNum);
    file.read(weightDescriptor);
    file.read(maxSpeed);
    file.read(minSpeed);
    file.read(acceleration);
    file.read(braking);
    file.read(grip);
    file.read(handling);
    bytes_read_ += 2 * 11;

    for (auto &i : remap24) {
        file.read(i.h);
        file.read(i.l);
        file.read(i.s);
    }
    bytes_read_ += 12 * 3 * 2;
    for (unsigned char &i : remap8)
        file.read(i);
    bytes_read_ += 12;

    file.read(vtype);
    file.read(model);
    file.read(turning);
    file.read(damagable);
    bytes_read_ += 4;

    for (unsigned short &i : value)
        file.read(i);
    bytes_read_ += 4 * 2;

    file.read(cx);
    file.read(cy);
    file.read(moment);
    bytes_read_ += 2 + 4;

    PHYSFS_uint32 fixed_tmp;
    file.read(fixed_tmp);
    // rbpMass = fixed_tmp / 65536

    file.read(fixed_tmp);
    // g1_Thrust = fixed_tmp / 65536

    file.read(fixed_tmp);
    // tyreAdhesionX = fixed_tmp / 65536

    file.read(fixed_tmp);
    // tyreAdhesionY = fixed_tmp / 65536

    file.read(fixed_tmp);
    // handBrakeFriction = fixed_tmp / 65536

    file.read(fixed_tmp);
    // footBrakeFriction = fixed_tmp / 65536

    file.read(fixed_tmp);
    // fronBrakeBias = fixed_tmp / 65536

    bytes_read_ += 7 * 4;

    file.read(turnRatio);
    file.read(driveWheelOffset);
    file.read(steeringWheelOffset);
    bytes_read_ += 3 * 2;

    file.read(fixed_tmp);
    // backEndSlideValue = fixed_tmp / 65536

    file.read(fixed_tmp);
    // handBrakeSlideValue = fixed_tmp / 65536

    bytes_read_ += 2 * 4;

    file.read(convertible);
    file.read(engine);
    file.read(radio);
    file.read(horn);
    file.read(soundFunction);
    file.read(fastChangeFlag);
    bytes_read_ += 6;

    file.read(numDoors);
    bytes_read_ += 2;
    if (numDoors > 2) {
        WARN("num-doors: {} > 2 ???", numDoors);
        numDoors = 0;
    }

    for (int i = 0; i < numDoors; i++) {
        file.read(door[i].rpy);
        file.read(door[i].rpx);
        file.read(door[i].object);
        file.read(door[i].delta);
        bytes_read_ += 4 * 2;
    }

    DEBUG("car of size {} is read ({} doors)", int(bytes_read_), int(numDoors));
}
