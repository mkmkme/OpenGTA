#pragma once

#include <cstdint>
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
    uint16_t size;
    unsigned char *ptr;
};

struct SpriteInfo {
    uint8_t w;
    uint8_t h;
    uint8_t deltaCount;
    uint16_t size;
    uint16_t clut;
    uint8_t xoffset;
    uint8_t yoffset;
    uint16_t page;
    // unsigned char* ptr;
    DeltaInfo delta[33]; // FIXME: GTA_SPRITE_MAX_DELTAS
};

} // namespace OpenGTA
