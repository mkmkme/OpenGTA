/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This software is provided as-is, without any express or implied       *
 * warranty. In no event will the authors be held liable for any         *
 * damages arising from the use of this software.                        *
 *                                                                       *
 * Permission is granted to anyone to use this software for any purpose, *
 * including commercial applications, and to alter it and redistribute   *
 * it freely, subject to the following restrictions:                     *
 *                                                                       *
 * 1. The origin of this software must not be misrepresented; you must   *
 * not claim that you wrote the original software. If you use this       *
 * software in a product, an acknowledgment in the product documentation *
 * would be appreciated but is not required.                             *
 *                                                                       *
 * 2. Altered source versions must be plainly marked as such, and must   *
 * not be misrepresented as being the original software.                 *
 *                                                                       *
 * 3. This notice may not be removed or altered from any source          *
 * distribution.                                                         *
 ************************************************************************/
#ifndef UTIL_IMAGE_LOADER_H
#define UTIL_IMAGE_LOADER_H

#include <span>
#include <string>
#include <vector>

#include "graphics/pagedtexture.h"

#include "base/config.h"

namespace ImageUtil {

/** Helper to calculate 2^k texture sizes.
 */
struct NextPowerOfTwo {
    NextPowerOfTwo(uint32_t _w, uint32_t _h)
    {
        w = 1;
        h = 1;
        while (w < _w) {
            w <<= 1;
        }
        while (h < _h) {
            h <<= 1;
        }
    }
    uint32_t w;
    uint32_t h;
};

/** Run scale2x on 32bit input image.
 */
std::vector<uint8_t> scale2x_32bit(std::span<const uint8_t> src, int src_width, int src_height);

std::vector<uint8_t> scale2x_24bit(std::span<const uint8_t> src, int src_width, int src_height);

using WidthHeightPair = std::pair<uint16_t, uint16_t>;
// hardcoded data for known images
WidthHeightPair lookupImageSize(const std::string &name, uint32_t size);
// load a rgb image
OpenGL::PagedTexture loadImageRAW(const std::string &name);
// load a palette image and guess the palette filename
// OpenGL::PagedTexture loadImageRAT(const std::string & name);
// load a palette image using palette file
OpenGL::PagedTexture loadImageRATWithPalette(const std::string &name, const std::string &palette_file);
OpenGL::PagedTexture loadImageSDL(const std::string &name);

extern bool mipmapTextures;
extern float supportedMaxAnisoDegree;

// plain simple garden-variety create-a-texture; needs to be 2^k
uint32_t createGLTexture(size_t w, size_t h, bool rgba, std::span<const uint8_t> pixels);

// blitting a buffer into another; no checks done!
void copyImage2Image(
    std::span<uint8_t> dest,
    std::span<const uint8_t> src,
    uint16_t srcWidth,
    uint16_t srcHeight,
    uint16_t destWidth
);

// texture-class instance from pixel data; does transform to 2^k if required
OpenGL::PagedTexture createEmbeddedTexture(size_t w, size_t h, bool rgba, std::vector<uint8_t> pixels);
} // namespace ImageUtil

#endif
