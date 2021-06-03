#include "loaded-anim.h"

#include "util/log.h"

OpenGTA::LoadedAnim::LoadedAnim(PHYSFS_file *fd)
{
    PHYSFS_readBytes(fd, static_cast<void *>(&block), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&which), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&speed), 1);
    PHYSFS_readBytes(fd, static_cast<void *>(&frameCount), 1);

    if (frameCount > 180) {
        ERROR << "found animation with " << int(frameCount) << " frames ???"
              << std::endl;
    }

    frame.reserve(frameCount);
    for (int i = 0; i < frameCount; i++) {
        PHYSFS_uint8 tmp = 0;
        PHYSFS_readBytes(fd, static_cast<void *>(&tmp), 1);
        frame.push_back(tmp);
    }
}
