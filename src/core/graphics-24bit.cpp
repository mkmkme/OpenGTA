/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *                                                                       *
 * This file contains code derived from information copyrighted by       *
 * DMA Design. It may not be used in a commercial product.               *
 *                                                                       *
 * See license.txt for details.                                          *
 *                                                                       *
 * This notice may not be removed or altered.                            *
 ************************************************************************/
#include <cassert>
#include <cstddef>

#include <core/graphics-24bit.h>
#include <core/sprite-info.h>

#include <util/log.h>

using namespace Util;
namespace OpenGTA {

#define GTA_GRAPHICS_GRY 325
#define GTA_GRAPHICS_G24 336

Graphics24Bit::Graphics24Bit(const std::string &style)
    : GraphicsBase(style)
{
    _topHeaderSize = 64;
    rawClut = nullptr;
    palIndex = nullptr;
    loadHeader();
    setupBlocking();
    // actually the next two are style003.g24, but at least somewhere close
    firstValidPedRemap = 60;
    lastValidPedRemap = 116;
    if (style.find("001") != std::string::npos) {
        firstValidPedRemap = 75;
        lastValidPedRemap = 131;
    } else if (style.find("002") != std::string::npos) {
        firstValidPedRemap = 79;
        lastValidPedRemap = 135;
    } else if (style.find("003") != std::string::npos) {
        // already set
    } else {
        WARN("Unknown g24 style - ped remaps most likely broken!");
    }
}

Graphics24Bit::~Graphics24Bit()
{
    delete[] rawClut;
    delete[] palIndex;
}

void Graphics24Bit::loadHeader()
{
    UInt32 vc;
    styleFile.read(vc);
    if (vc != GTA_GRAPHICS_G24) {
        ERROR("graphics file specifies version {} (should be {})", vc, GTA_GRAPHICS_G24);
        return;
    }
    styleFile.read(sideSize);
    styleFile.read(lidSize);
    styleFile.read(auxSize);
    styleFile.read(animSize);
    styleFile.read(clutSize);
    styleFile.read(tileclutSize);
    styleFile.read(spriteclutSize);
    styleFile.read(newcarclutSize);
    styleFile.read(fontclutSize);
    styleFile.read(paletteIndexSize);
    styleFile.read(objectInfoSize);
    styleFile.read(carInfoSize);
    styleFile.read(spriteInfoSize);
    styleFile.read(spriteGraphicsSize);
    styleFile.read(spriteNumberSize);

    /*
        INFO << "Version: " << vc << std::endl << " Block textures: S " << sideSize / 4096 << " L " <<
          lidSize / 4096 << " A " << auxSize / 4096 << std::endl;
          */
    if (sideSize % 4096 != 0) {
        ERROR("Side-Block texture size is not a multiple of 4096");
        return;
    }
    if (lidSize % 4096 != 0) {
        ERROR("Lid-Block texture size is not a multiple of 4096");
        return;
    }
    if (auxSize % 4096 != 0) {
        ERROR("Aux-Block texture size is not a multiple of 4096");
        return;
    }

    UInt32 tmp = sideSize / 4096 + lidSize / 4096 + auxSize / 4096;
    tmp = tmp % 4;
    if (tmp) {
        auxBlockTrailSize = (4 - tmp) * 4096;
        INFO("adjusting aux-block by {}", auxBlockTrailSize);
    }
    INFO("Anim size: {}", animSize);
    INFO(
        "Obj-info size: {} car-size: {} sprite-info size: {} graphic size: {} "
        "numbers s: {}",
        objectInfoSize,
        carInfoSize,
        spriteNumberSize,
        spriteGraphicsSize,
        spriteNumberSize
    );
    if (spriteNumberSize != 42) {
        ERROR("spriteNumberSize is {} (should be 42)", spriteNumberSize);
        return;
    }

    INFO(
        "clut: {} tileclut: {} spriteclut: {} newcar: {} fontclut: {}",
        clutSize,
        tileclutSize,
        spriteclutSize,
        newcarclutSize,
        fontclutSize
    );
    INFO("pal-index size: {}", paletteIndexSize);

    loadTileTextures();
    loadAnim();
    loadClut();
    loadPalIndex();
    loadObjectInfo();
    loadCarInfo();
    loadSpriteInfo();
    loadSpriteGraphics();
    loadSpriteNumbers();
}

void Graphics24Bit::loadClut()
{
    UInt64 st =
        static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize + animSize;
    styleFile.ensurePosition(st);
    pagedClutSize = clutSize;
    if (clutSize % 65536 != 0)
        pagedClutSize += (65536 - (clutSize % 65536));
    rawClut = new unsigned char[pagedClutSize];
    assert(rawClut);
    styleFile.read(rawClut, pagedClutSize);
}

void Graphics24Bit::loadPalIndex()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + pagedClutSize;
    styleFile.ensurePosition(st);
    UInt16 pal_index_count = paletteIndexSize / 2;
    assert(paletteIndexSize % 2 == 0);
    palIndex = new UInt16[pal_index_count];
    for (UInt16 i = 0; i < pal_index_count; i++) {
        styleFile.read(palIndex[i]);
    }
}

void Graphics24Bit::loadCarInfo()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + pagedClutSize + paletteIndexSize + objectInfoSize;
    // INFO("seek for {}", st);
    loadCarInfo_shared(st);
}

void Graphics24Bit::loadSpriteInfo()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + pagedClutSize + paletteIndexSize + objectInfoSize + carInfoSize;
    styleFile.ensurePosition(st);

    UInt8 v;
    UInt32 w;
    UInt32 _bytes_read = 0;
    while (_bytes_read < spriteInfoSize) {
        SpriteInfo si;
        styleFile.read(si.w);
        styleFile.read(si.h);
        styleFile.read(si.deltaCount);
        styleFile.read(v);
        styleFile.read(si.size);
        _bytes_read += 6;
        styleFile.read(si.clut);
        styleFile.read(si.xoffset);
        styleFile.read(si.yoffset);
        styleFile.read(si.page);
        _bytes_read += 6;
        /*
        std::cout << "sprite: " << int(si->w) << "x" << int(si->h) << " deltas: " << int(si->deltaCount)
        << " clut: " << si->clut << " x: " << int(si->xoffset) << " y: " << int(si->yoffset) <<
        " page: " << si->page << std::endl;
        */
        // sanity check
        if (v)
            WARN("Compression flag active in sprite!");
        if (int(si.w) * int(si.h) != int(si.size)) {
            ERROR("Sprite info size mismatch: {}x{} != {}", int(si.w), int(si.h), si.size);
            return;
        }
        if (si.deltaCount > 32) {
            ERROR("Delta count of sprite is {}", si.deltaCount);
            return;
        }
        for (UInt8 j = 0; j < si.deltaCount; j++) {
            si.delta[j].size = 0;
            si.delta[j].ptr = nullptr;
            if (si.deltaCount && (j < si.deltaCount)) {
                styleFile.read(si.delta[j].size);
                styleFile.read(w);
                _bytes_read += 6;
                si.delta[j].ptr = reinterpret_cast<unsigned char *>(w);
            }
        }
        spriteInfos.emplace_back(si);
    }
    st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
        pagedClutSize + paletteIndexSize + objectInfoSize + carInfoSize + spriteInfoSize;
    styleFile.ensurePosition(st);
}

void Graphics24Bit::loadSpriteNumbers()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + pagedClutSize + paletteIndexSize + objectInfoSize + carInfoSize + spriteInfoSize +
        spriteGraphicsSize;
    loadSpriteNumbers_shared(st);
}

void Graphics24Bit::loadSpriteGraphics()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + pagedClutSize + paletteIndexSize + objectInfoSize + carInfoSize + spriteInfoSize;
    styleFile.ensurePosition(st);

    rawSprites.resize(spriteGraphicsSize);
    styleFile.read(rawSprites.data(), rawSprites.size());

    auto i = spriteInfos.begin();
    auto end = spriteInfos.end();
    UInt32 _pagewise = 256 * 256;
    while (i != end) {
        SpriteInfo &info = *i;
        for (uint8_t k = 0; k < info.deltaCount; ++k) {
            const auto offset = reinterpret_cast<uintptr_t>(info.delta[k].ptr);
            const auto page = offset / 65536;
            const auto y = (offset % 65536) / 256;
            const auto x = (offset % 65536) % 256;
            info.delta[k].ptr = rawSprites.data() + page * _pagewise + 256 * y + x;
        }
        i++;
    }
}

void Graphics24Bit::loadObjectInfo()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + pagedClutSize + paletteIndexSize;
    loadObjectInfo_shared(st);
}

void Graphics24Bit::applyClut(
    unsigned char *src,
    unsigned char *dst,
    const size_t &len,
    UInt16 clutIdx,
    bool rgba
)
{
    UInt32 off = 65536 * (clutIdx / 64) + 4 * (clutIdx % 64);
    for (size_t i = 0; i < len; i++) {
        UInt32 coff = UInt32(*src) * 256 + off;
        *dst = rawClut[coff + 2];
        ++dst;
        *dst = rawClut[coff + 1];
        ++dst;
        *dst = rawClut[coff + 0];
        ++dst;
        if (rgba) {
            if (*src == 0)
                *dst = 0;
            else
                *dst = 0xff;
            ++dst;
        }
        ++src;
    }
}

std::span<const UInt8> Graphics24Bit::getLid(UInt8 idx, unsigned int /*not_used*/, bool rgba)
{
    prepareLidTexture(idx - 1, tileTmp);
    unsigned char *src = tileTmp.data();
    unsigned char *dst = (rgba) ? tileTmpRGBA.data() : tileTmpRGB.data();
    UInt16 clutIdx = palIndex[static_cast<size_t>(4 * (idx + sideSize / 4096))];
    applyClut(src, dst, 4096, clutIdx, rgba);

    if (rgba)
        return tileTmpRGBA;
    return tileTmpRGB;
}

std::span<const UInt8> Graphics24Bit::getSide(UInt8 idx, unsigned int /*not_used*/, bool rgba)
{
    prepareSideTexture(idx - 1, tileTmp);
    unsigned char *src = tileTmp.data();
    unsigned char *dst = (rgba) ? tileTmpRGBA.data() : tileTmpRGB.data();
    UInt16 clutIdx = palIndex[static_cast<size_t>(idx * 4)];
    applyClut(src, dst, 4096, clutIdx, rgba);

    if (rgba)
        return tileTmpRGBA;
    return tileTmpRGB;
}

std::span<const UInt8> Graphics24Bit::getAux(UInt8 idx, unsigned int /*not_used*/, bool rgba)
{
    prepareAuxTexture(idx - 1, tileTmp);

    unsigned char *src = tileTmp.data();
    unsigned char *dst = (rgba) ? tileTmpRGBA.data() : tileTmpRGB.data();
    UInt16 clutIdx = palIndex[4 * (idx + sideSize / 4096 + lidSize / 4096)];
    applyClut(src, dst, 4096, clutIdx, rgba);

    if (rgba)
        return tileTmpRGBA;
    return tileTmpRGB;
}

std::vector<UInt8> Graphics24Bit::getSpriteBitmap(size_t id, int remap, UInt32 delta)
{
    const SpriteInfo &info = spriteInfos[id];
    const UInt32 y = info.yoffset;
    const UInt32 x = info.xoffset;
    const UInt32 page_size = 256 * 256;

    auto *page_start = rawSprites.data() + static_cast<size_t>(info.page * page_size);

    std::vector<UInt8> result(page_size);
    memcpy(result.data(), page_start, page_size);
    if (delta > 0) {
        handleDeltas(info, result.data(), delta);
        /*
        assert(delta < info->deltaCount);
        DeltaInfo & di = info->delta[delta];
        applyDelta(*info, dest+256*y+x, di);
        */
    }

    std::vector<UInt8> bigbuf(static_cast<size_t>(page_size) * 4);
    auto *bigbuf_raw = bigbuf.data();
    auto *result_raw = result.data();
    unsigned int skip_cluts = 0;
    if (remap > -1)
        skip_cluts = spriteclutSize / 1024 + remap + 1;

    UInt16 clutIdx = palIndex[info.clut + tileclutSize / 1024] + skip_cluts;
    //  UInt16 clutIdx = palIndex[info->clut + (spriteclutSize + tileclutSize) / 1024] + (remap > -1 ? remap+2 :
    //  0);
    applyClut(result.data(), bigbuf_raw, page_size, clutIdx, true);
    assert(page_size > UInt32(info.w * info.h * 4));
    for (uint16_t i = 0; i < info.h; i++) {
        memcpy(result_raw, bigbuf_raw + static_cast<size_t>((256 * y + x) * 4), static_cast<size_t>(info.w * 4));
        result_raw += static_cast<ptrdiff_t>(info.w * 4);
        bigbuf_raw += static_cast<ptrdiff_t>(256 * 4);
    }

    return result;
}
} // namespace OpenGTA
