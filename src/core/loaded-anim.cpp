#include <core/loaded-anim.h>

#include "util/file-manager.h"
#include <util/log.h>

OpenGTA::LoadedAnim::LoadedAnim(Util::PhysFSFile &styleFile)
{
    styleFile.read(block);
    styleFile.read(which);
    styleFile.read(speed);
    styleFile.read(frameCount);

    if (frameCount > 180)
        ERROR("found animation with {} frames ???", frameCount);

    frame.reserve(frameCount);
    for (int i = 0; i < frameCount; i++) {
        UInt8 tmp = 0;
        styleFile.read(tmp);
        frame.push_back(tmp);
    }
}
