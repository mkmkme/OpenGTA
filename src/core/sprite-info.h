#pragma once

#include <core/numeric-types.h>

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
    UInt16 size;
    unsigned char *ptr;
};

struct SpriteInfo {
    UInt8 w;
    UInt8 h;
    UInt8 deltaCount;
    UInt16 size;
    UInt16 clut;
    UInt8 xoffset;
    UInt8 yoffset;
    UInt16 page;
    // unsigned char* ptr;
    DeltaInfo delta[33]; // FIXME: GTA_SPRITE_MAX_DELTAS
};

} // namespace OpenGTA
