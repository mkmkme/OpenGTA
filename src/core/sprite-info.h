#pragma once

#include <physfs.h>

namespace OpenGTA {

/*
 * float->fixed:
 *  fixed = int(floatnum * 65536)
 *
 * fixed->float
 *  float = float(fixedNum)/65536
 *
 *  int->fixed
 *   fixed = intNum << 16
 *
 *  fixed->int
 *   int = fixedNum >> 16
 */

struct DeltaInfo {
    PHYSFS_uint16 size;
    unsigned char *ptr;
};

struct SpriteInfo {
    PHYSFS_uint8 w;
    PHYSFS_uint8 h;
    PHYSFS_uint8 deltaCount;
    PHYSFS_uint16 size;
    PHYSFS_uint16 clut;
    PHYSFS_uint8 xoffset;
    PHYSFS_uint8 yoffset;
    PHYSFS_uint16 page;
    // unsigned char* ptr;
    DeltaInfo delta[33]; // FIXME: GTA_SPRITE_MAX_DELTAS
};

} // namespace OpenGTA
