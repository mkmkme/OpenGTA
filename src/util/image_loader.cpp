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
#include "util/image_loader.h"

#include <cassert>
#include <cstdint>

#include <SDL_image.h>
#include <physfs.h>

#include <SDL2/SDL_surface.h> // WITH_SDL_IMAGE

#include "util/errors.h"
#include "util/file-manager.h"
#include "util/log.h"
#include "util/physfsrwops.h"
#include "util/string_helpers.h"

#include "core/graphics-8bit.h"
#ifdef _WIN32
#include <Windows.h>
#elif defined(__APPLE__)
#include <OpenGL/glext.h>
#else
#include <GL/glext.h>
#endif
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

namespace ImageUtil {
using OpenGL::PagedTexture;

WidthHeightPair lookupImageSize(const std::string &name, const uint32_t size)
{
    std::string iname { Util::string_upper(name) };
    uint16_t width = 0;
    uint16_t height = 0;
    uint32_t bpp = 0;

#define SET_SIZE(w, h) \
    width = w;         \
    height = h;

    // m4 tools/raw_images.m4
    if ((iname.find("CUT") == 0) && (iname.find(".RA") == 4)) {
        SET_SIZE(640, 480);
    }
    if ((iname.find("F_BMG.RA") == 0)) {
        SET_SIZE(100, 50);
    }
    if ((iname.find("F_DMA.RA") == 0)) {
        SET_SIZE(78, 109);
    }
    if ((iname.find("F_LOGO") == 0) && (iname.find(".RA") == 7)) {
        SET_SIZE(640, 168);
    }
    if ((iname.find("F_LOWER") == 0) && (iname.find(".RA") == 8)) {
        SET_SIZE(640, 312);
    }
    if ((iname.find("F_PLAYN.RA") == 0)) {
        SET_SIZE(180, 50);
    }
    if ((iname.find("F_PLAY") == 0) && (iname.find(".RA") == 7)) {
        SET_SIZE(102, 141);
    }
    if ((iname.find("F_UPPER.RA") == 0)) {
        SET_SIZE(640, 168);
    }
#undef SET_SIZE

    // end-of-generated code
    if (iname.find(".RAW") == iname.length() - 4)
        bpp = 3;

    if (iname.find(".RAT") == iname.length() - 4)
        bpp = 1;

    if (!bpp || bpp * width * height != size)
        ERROR("could not identify image: {} size: {}", name, size);
    return std::make_pair(width, height);
}

OpenGL::PagedTexture loadImageRAW(const std::string &name)
{
    Util::PhysFSFile pf { name };

    const auto nbytes = pf.length();

    WidthHeightPair whp = lookupImageSize(name, nbytes);

    if (whp.first == 0 || whp.second == 0) {
        WARN("aborting image load");
        throw Util::UnknownKey(name + " - RAW file size unknown");
    }

    std::vector<uint8_t> buffer(nbytes);
    pf.read(buffer.data(), buffer.size());

    return createEmbeddedTexture(whp.first, whp.second, false, std::move(buffer));
}

OpenGL::PagedTexture loadImageRATWithPalette(const std::string &name, const std::string &palette_file)
{

    Util::PhysFSFile pf { name };
    const auto nbytes = pf.length();

    const auto whp = lookupImageSize(name, nbytes);
    if (whp.first == 0 || whp.second == 0) {
        WARN("aborting image load");
        throw Util::UnknownKey(name + " - RAT file size unknown");
    }
    std::vector<uint8_t> lb1(nbytes);
    pf.read(lb1.data(), lb1.size());

    pf = Util::PhysFSFile { palette_file };
    OpenGTA::Graphics8Bit::RGBPalette rgb { pf };

    std::vector<uint8_t> lb2(nbytes * 3);
    rgb.apply(nbytes, lb1.data(), lb2.data(), false);

    return createEmbeddedTexture(whp.first, whp.second, false, std::move(lb2));
}

#ifdef OGTA_WITH_SDL_IMAGE
OpenGL::PagedTexture loadImageSDL(const std::string &name)
{
    SDL_RWops *rwops = PHYSFSRWOPS_openRead(name.c_str());
    SDL_Surface *surface = IMG_Load_RW(rwops, 1);
    assert(surface != nullptr);

    NextPowerOfTwo npot(surface->w, surface->h);
    uint16_t bpp = surface->format->BytesPerPixel;

    std::vector<uint8_t> buffer(npot.w * npot.h * bpp);
    SDL_LockSurface(surface);
    std::span<uint8_t> pixels_span { static_cast<uint8_t *>(surface->pixels),
                                     static_cast<size_t>(surface->pitch * surface->h) };
    copyImage2Image(buffer, pixels_span, surface->pitch, surface->h, npot.w * bpp);
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(surface);

    GLuint texture = createGLTexture(npot.w, npot.h, bpp == 4, buffer);
    return OpenGL::PagedTexture(texture, 0, 0, GLfloat(surface->w) / npot.w, GLfloat(surface->h) / npot.h);
}
#endif

#define GL_SILENCE_DEPRECATION
uint32_t createGLTexture(size_t w, size_t h, bool rgba, std::span<const uint8_t> pixels)
{
    GLuint tex;
    fmt::println("before glGenTextures");
    glGenTextures(1, &tex);
    fmt::println("after glGenTextures");
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    if (!mipmapTextures)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (rgba) {
        if (mipmapTextures)
            gluBuild2DMipmaps(GL_TEXTURE_2D, 4, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    } else {
        if (mipmapTextures)
            gluBuild2DMipmaps(GL_TEXTURE_2D, 3, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    }
    if (supportedMaxAnisoDegree > 1.0f)
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &supportedMaxAnisoDegree);

    GL_CHECKERROR;
    return tex;
}

void copyImage2Image(
    std::span<uint8_t> dest,
    std::span<const uint8_t> src,
    uint16_t srcWidth,
    uint16_t srcHeight,
    uint16_t destWidth
)
{
    auto d = dest.begin();
    uint32_t srcOff = 0;
    for (uint16_t j = 0; j < srcHeight; ++j) {
        std::ranges::copy_n(src.begin() + srcOff, srcWidth, d);
        srcOff += srcWidth;
        d += destWidth;
    }
}

OpenGL::PagedTexture createEmbeddedTexture(size_t w, size_t h, bool rgba, std::vector<uint8_t> pixels)
{

    NextPowerOfTwo npot(w, h);
    std::vector<uint8_t> buff;

    if (npot.w != uint32_t(w) || npot.h != uint32_t(h)) {
        uint32_t bpp = (rgba ? 4 : 3);
        uint32_t bufSize = npot.w * npot.h * bpp;
        std::vector<uint8_t> tmp(bufSize);
        copyImage2Image(tmp, pixels, w * bpp, h, npot.w * bpp);
        buff = std::move(tmp);
    }

    GLuint tex = createGLTexture(npot.w, npot.h, rgba, buff);
    return PagedTexture(tex, 0, 0, float(w) / npot.w, float(h) / npot.h);
}

namespace {

inline int readInt24(const uint8_t *x) noexcept
{
    return (x[0] << 16) | (x[1] << 8) | x[2];
}

inline void writeInt24(uint8_t *x, int i) noexcept
{
    x[0] = i >> 16;
    x[1] = (i >> 8) & 0xff;
    x[2] = i & 0xff;
}

} // namespace

// Converting
//    B
//  D E F
//    H
// To
//   E0 E1
//   E2 E3
std::vector<uint8_t> scale2x_24bit(std::span<const uint8_t> src, const int src_width, const int src_height)
{
    const int srcpitch = src_width * 3;
    const int dstpitch = src_width * 6;

    std::vector<uint8_t> dstpix(src_width * src_height * 3 * 4);
    const auto *srcraw = src.data();
    auto *dstpixraw = dstpix.data();
    int E0, E1, E2, E3, B, D, E, F, H;
    for (int looph = 0; looph < src_height; ++looph) {
        for (int loopw = 0; loopw < src_width; ++loopw) {
            B = readInt24(srcraw + (std::max(0, looph - 1) * srcpitch) + (3 * loopw));
            D = readInt24(srcraw + (looph * srcpitch) + (3 * std::max(0, loopw - 1)));
            E = readInt24(srcraw + (looph * srcpitch) + (3 * loopw));
            F = readInt24(srcraw + (looph * srcpitch) + (3 * std::min(src_width - 1, loopw + 1)));
            H = readInt24(srcraw + (std::min(src_height - 1, looph + 1) * srcpitch) + (3 * loopw));

            E0 = D == B && B != F && D != H ? D : E;
            E1 = B == F && B != D && F != H ? F : E;
            E2 = D == H && D != B && H != F ? D : E;
            E3 = H == F && D != H && B != F ? F : E;

            writeInt24((dstpixraw + (looph * 2 * dstpitch) + (loopw * 2 * 3)), E0);
            writeInt24((dstpixraw + (looph * 2 * dstpitch) + ((loopw * 2 + 1) * 3)), E1);
            writeInt24((dstpixraw + ((looph * 2 + 1) * dstpitch) + (loopw * 2 * 3)), E2);
            writeInt24((dstpixraw + ((looph * 2 + 1) * dstpitch) + ((loopw * 2 + 1) * 3)), E3);
        }
    }
    return dstpix;
}

std::vector<uint8_t> scale2x_32bit(std::span<const uint8_t> src, const int src_width, const int src_height)
{
    const int srcpitch = src_width * 4;
    const int dstpitch = src_width * 8;

    std::vector<uint8_t> dstpix(src_width * src_height * 4 * 4);
    auto *dstpixraw = dstpix.data();
    const auto *srcraw = src.data();
    uint32_t E0, E1, E2, E3, B, D, E, F, H;
    for (int looph = 0; looph < src_height; ++looph) {
        for (int loopw = 0; loopw < src_width; ++loopw) {
            B = *(uint32_t *) (srcraw + (std::max(0, looph - 1) * srcpitch) + (4 * loopw));
            D = *(uint32_t *) (srcraw + (looph * srcpitch) + (4 * std::max(0, loopw - 1)));
            E = *(uint32_t *) (srcraw + (looph * srcpitch) + (4 * loopw));
            F = *(uint32_t *) (srcraw + (looph * srcpitch) + (4 * std::min(src_width - 1, loopw + 1)));
            H = *(uint32_t *) (srcraw + (std::min(src_height - 1, looph + 1) * srcpitch) + (4 * loopw));

            E0 = D == B && B != F && D != H ? D : E;
            E1 = B == F && B != D && F != H ? F : E;
            E2 = D == H && D != B && H != F ? D : E;
            E3 = H == F && D != H && B != F ? F : E;

            *(uint32_t *) (dstpixraw + (looph * 2 * dstpitch) + (loopw * 2 * 4)) = E0;
            *(uint32_t *) (dstpixraw + (looph * 2 * dstpitch) + ((loopw * 2 + 1) * 4)) = E1;
            *(uint32_t *) (dstpixraw + ((looph * 2 + 1) * dstpitch) + (loopw * 2 * 4)) = E2;
            *(uint32_t *) (dstpixraw + ((looph * 2 + 1) * dstpitch) + ((loopw * 2 + 1) * 4)) = E3;
        }
    }
    return dstpix;
}

bool mipmapTextures = false;
float supportedMaxAnisoDegree = 1.0f;
} // namespace ImageUtil
