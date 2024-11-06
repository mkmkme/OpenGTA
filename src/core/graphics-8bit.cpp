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
#include <memory>

#include <core/graphics-8bit.h>
#include <core/sprite-info.h>

#include <util/errors.h>
#include <util/file-manager.h>
#include <util/file_helper.h>
#include <util/log.h>

using namespace Util;

namespace OpenGTA {

#define GTA_GRAPHICS_GRY 325
#define GTA_GRAPHICS_G24 336

Graphics8Bit::Graphics8Bit(const std::string &style)
    : GraphicsBase(style)
{
    _topHeaderSize = 52;
    auxBlockTrailSize = 0;
    loadHeader();
    setupBlocking();
    firstValidPedRemap = 131;
    lastValidPedRemap = 187;
}

void Graphics8Bit::dump()
{

    uint32_t gs = sideSize + lidSize + auxSize;

    INFO("* graphics info *");
    INFO("{} bytes in {} pages {} images", gs, gs / 65536, gs / 4096);
    INFO("{} sprites ({}) total: {} bytes", spriteInfos.size(), spriteInfoSize, spriteGraphicsSize);
    INFO("sprite numbers:");
    INFO("{} arrows", spriteNumbers.GTA_SPRITE_ARROW);
    INFO("{} digits", spriteNumbers.GTA_SPRITE_DIGITS);
    INFO("{} boats", spriteNumbers.GTA_SPRITE_BOAT);
    INFO("{} boxes", spriteNumbers.GTA_SPRITE_BOX);
    INFO("{} buses", spriteNumbers.GTA_SPRITE_BUS);
    INFO("{} cars", spriteNumbers.GTA_SPRITE_CAR);
    INFO("{} objects", spriteNumbers.GTA_SPRITE_OBJECT);
    INFO("{} peds", spriteNumbers.GTA_SPRITE_PED);
    INFO("{} speedos", spriteNumbers.GTA_SPRITE_SPEEDO);
    INFO("{} tanks", spriteNumbers.GTA_SPRITE_TANK);
    INFO("{} traffic lights", spriteNumbers.GTA_SPRITE_TRAFFIC_LIGHTS);
    INFO("{} trains", spriteNumbers.GTA_SPRITE_TRAIN);
    INFO("{} train doors", spriteNumbers.GTA_SPRITE_TRDOORS);
    INFO("{} bikes", spriteNumbers.GTA_SPRITE_BIKE);
    INFO("{} trams", spriteNumbers.GTA_SPRITE_TRAM);
    INFO("{} wbuses", spriteNumbers.GTA_SPRITE_WBUS);
    INFO("{} wcars", spriteNumbers.GTA_SPRITE_WCAR);
    INFO("{} exes", spriteNumbers.GTA_SPRITE_EX);
    INFO("{} tumcars", spriteNumbers.GTA_SPRITE_TUMCAR);
    INFO("{} tumtrucks", spriteNumbers.GTA_SPRITE_TUMTRUCK);
    INFO("{} ferries", spriteNumbers.GTA_SPRITE_FERRY);
    INFO("#object-info: {} #car-info: {}", objectInfos.size(), carInfos.size());
}

void Graphics8Bit::loadHeader()
{
    UInt32 vc;
    styleFile.read(vc);
    if (vc != GTA_GRAPHICS_GRY) {
        ERROR("graphics file specifies version {} instead of {}", vc, GTA_GRAPHICS_GRY);
        throw Util::InvalidFormat("8-bit loader failed");
    }
    styleFile.read(sideSize);
    styleFile.read(lidSize);
    styleFile.read(auxSize);
    styleFile.read(animSize);
    styleFile.read(paletteSize);
    styleFile.read(remapSize);
    styleFile.read(remapIndexSize);
    styleFile.read(objectInfoSize);
    styleFile.read(carInfoSize);
    styleFile.read(spriteInfoSize);
    styleFile.read(spriteGraphicsSize);
    styleFile.read(spriteNumberSize);

    INFO("Block textures: S {} L {} A {}", sideSize / 4096, lidSize / 4096, auxSize / 4096);
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
    INFO(
        "Anim size: {} palette size: {} remap size: {} remap-index size: {}",
        animSize,
        paletteSize,
        remapSize,
        remapIndexSize
    );
    INFO(
        "Obj-info size: {} car-size: {} sprite-info size: {} graphic size: {} "
        "numbers s: {}",
        objectInfoSize,
        carInfoSize,
        spriteInfoSize,
        spriteGraphicsSize,
        spriteNumberSize
    );
    if (spriteNumberSize != 42) {
        ERROR("spriteNumberSize is {} (should be 42)", spriteNumberSize);
        return;
    }
    loadTileTextures();
    loadAnim();
    loadPalette();
    loadRemapTables();
    loadRemapIndex();
    loadObjectInfo();
    loadCarInfo();
    loadSpriteInfo();
    loadSpriteGraphics();
    loadSpriteNumbers();
    dump();
}

void Graphics8Bit::loadPalette()
{
    UInt64 st =
        static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize + animSize;
    styleFile.ensurePosition(st);
    masterRGB_ = std::make_unique<RGBPalette>(styleFile);
}

void Graphics8Bit::loadRemapTables()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + paletteSize;
    styleFile.ensurePosition(st);
    styleFile.read(remapTables, sizeof(remapTables));
    /*
    for (int i=0; i < 256; i++) {
      for (int j = 0; j < 256; j++) {
        std::cout << int(remapTables[i][j]) << " ";
      }
      std::cout << std::endl;
    }*/
}

void Graphics8Bit::loadRemapIndex()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + paletteSize + remapSize;
    styleFile.ensurePosition(st);
    styleFile.read(remapIndex, sizeof(remapIndex));
    /*
    std::cout << "LID remap tables" << std::endl;
    for (int i=0; i<256; ++i) {
      std::cout << i << ": " << int(remapIndex[i][0]) << ", " << int(remapIndex[i][1]) <<
        ", " << int(remapIndex[i][2]) << ", " << int(remapIndex[i][3]) << std::endl;
    }*/
}

void Graphics8Bit::loadObjectInfo()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + paletteSize + remapSize + remapIndexSize;
    loadObjectInfo_shared(st);
}

void Graphics8Bit::loadCarInfo()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + paletteSize + remapSize + remapIndexSize + objectInfoSize;
    loadCarInfo_shared(st);
}

void Graphics8Bit::loadSpriteInfo()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + paletteSize + remapSize + remapIndexSize + objectInfoSize + carInfoSize;
    styleFile.ensurePosition(st);

    UInt8 compressionFlag;
    UInt32 w;
    UInt32 _bytes_read = 0;
    while (_bytes_read < spriteInfoSize) {
        SpriteInfo si;
        styleFile.read(si.w);
        styleFile.read(si.h);
        styleFile.read(si.deltaCount);
        styleFile.read(compressionFlag);
        styleFile.read(si.size);
        styleFile.read(w);
        _bytes_read += 10;
        // si->ptr = reinterpret_cast<unsigned char*>(w);
        si.page = w / 65536;
        si.xoffset = (w % 65536) % 256;
        si.yoffset = (w % 65536) / 256;
        si.clut = 0;

        // sanity check
        if (compressionFlag)
            WARN("Compression flag active in sprite!");
        if (int(si.w) * int(si.h) != int(si.size)) {
            ERROR("Sprite info size mismatch: {}x{} != {}", int(si.w), int(si.h), si.size);
            return;
        }
        if (si.deltaCount > 32) {
            ERROR("Delta count of sprite is {} (should be <= 32)", si.deltaCount);
            return;
        }
        for (UInt8 j = 0; j < 33; ++j) {
            si.delta[j].size = 0;
            si.delta[j].ptr = nullptr;
            if (si.deltaCount && (j < si.deltaCount)) {
                // std::cout << "reading " << int(j) << std::endl;
                styleFile.read(si.delta[j].size);
                styleFile.read(w);
                _bytes_read += 6;
                si.delta[j].ptr = reinterpret_cast<unsigned char *>(w);
            }
        }
        spriteInfos.push_back(si);
    }
    st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
        paletteSize + remapSize + remapIndexSize + objectInfoSize + carInfoSize + spriteInfoSize;
    styleFile.ensurePosition(st);
}

void Graphics8Bit::loadSpriteGraphics()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + paletteSize + remapSize + remapIndexSize + objectInfoSize + carInfoSize + spriteInfoSize;
    styleFile.ensurePosition(st);
    rawSprites.resize(spriteGraphicsSize);
    styleFile.read(rawSprites.data(), rawSprites.size());

    if (spriteInfos.empty()) {
        INFO("No SpriteInfo post-loading work done - structure is empty");
        return;
    }
    auto i = spriteInfos.begin();
    auto end = spriteInfos.end();
    UInt32 _pagewise = 256 * 256;
    while (i != end) {
        SpriteInfo &info = *i;
        /*
        UInt32 offset = reinterpret_cast<UInt32>(info->ptr);
        UInt32 page = offset / 65536;
        UInt32 y = (offset % 65536) / 256;
        UInt32 x = (offset % 65536) % 256;
        */
        // std::cout << int(info->w) << "x" << int(info->h) << " " << int(info->deltaCount) << " deltas" << std::endl;
        // std::cout << offset << " page " << page << " x,y " << x <<","<<y<< std::endl;
        // info->ptr = rawSprites + page * _pagewise + 256 * y + x;
        for (uint8_t k = 0; k < info.deltaCount; ++k) {
            const auto tmp = reinterpret_cast<uintptr_t>(info.delta[k].ptr);
            const auto offset = static_cast<UInt32>(tmp);
            const auto page = offset / 65536;
            const auto y = (offset % 65536) / 256;
            const auto x = (offset % 65536) % 256;
            info.delta[k].ptr = rawSprites.data() + page * _pagewise + 256 * y + x;
        }
        i++;
    }
}

void Graphics8Bit::loadSpriteNumbers()
{
    UInt64 st = static_cast<UInt64>(_topHeaderSize) + sideSize + lidSize + auxSize + auxBlockTrailSize +
        animSize + paletteSize + remapSize + remapIndexSize + objectInfoSize + carInfoSize + spriteInfoSize +
        spriteGraphicsSize;
    loadSpriteNumbers_shared(st);
}

std::vector<UInt8> Graphics8Bit::getSpriteBitmap(size_t id, int remap, UInt32 delta)
{
    const SpriteInfo &info = spriteInfos[id];
    // UInt32 offset = reinterpret_cast<UInt32>(info->ptr);
    // const UInt32 page = offset / 65536;
    const UInt32 y = info.yoffset; // (offset % 65536) / 256;
    const UInt32 x = info.xoffset; // (offset % 65536) % 256;
    constexpr UInt32 page_size = 256 * 256;

    unsigned char *page_start = rawSprites.data() + static_cast<size_t>(info.page * page_size); // + 256 * y + x;
    assert(page_start != nullptr);

    std::vector<UInt8> result(page_size);

    unsigned char *result_raw = result.data();
    memcpy(result_raw, page_start, page_size);
    if (delta > 0) {
        handleDeltas(info, result_raw, delta);
        /*
        assert(delta < info->deltaCount);
        DeltaInfo & di = info->delta[delta];
        applyDelta(*info, result+256*y+x, di);
        */
    }
    if (remap > -1)
        applyRemap(page_size, remap, result_raw);
    std::vector<UInt8> bigbuf(static_cast<size_t>(page_size * 4));
    auto *bigbuf_raw = bigbuf.data();

    masterRGB_->apply(page_size, result_raw, bigbuf_raw, true);
    assert(page_size > UInt32(info.w * info.h * 4));
    for (uint16_t i = 0; i < info.h; i++) {
        memcpy(result_raw, bigbuf_raw + static_cast<size_t>((256 * y + x) * 4), static_cast<size_t>(info.w * 4));
        result_raw += static_cast<ptrdiff_t>(info.w * 4);
        bigbuf_raw += static_cast<ptrdiff_t>(256 * 4);
    }

    return result;
}

void Graphics8Bit::applyRemap(unsigned int len, unsigned int which, unsigned char *buffer)
{
    assert(buffer != nullptr);
    unsigned char *t = buffer;
    for (unsigned int i = 0; i < len; ++i) {
        *t = remapTables[which][*t]; // FIXME: is this the right order? Is this correct at all?
        t++;
    }
}

std::span<const UInt8> Graphics8Bit::getSide(UInt8 idx, unsigned int /*palIdx*/, bool rgba)
{
    prepareSideTexture(idx - 1, tileTmp);
    if (rgba) {
        masterRGB_->apply(4096, tileTmp.data(), tileTmpRGBA.data(), true);
        return tileTmpRGBA;
    }
    masterRGB_->apply(4096, tileTmp.data(), tileTmpRGB.data(), false);
    return tileTmpRGB;
}

std::span<const UInt8> Graphics8Bit::getLid(UInt8 idx, unsigned int palIdx, bool rgba)
{
    prepareLidTexture(idx - 1, tileTmp);
    if (palIdx > 0)
        applyRemap(4096, palIdx, tileTmp.data());

    if (rgba) {
        masterRGB_->apply(4096, tileTmp.data(), tileTmpRGBA.data(), true);
        return tileTmpRGBA;
    }
    masterRGB_->apply(4096, tileTmp.data(), tileTmpRGB.data(), false);
    return tileTmpRGB;
}

std::span<const UInt8> Graphics8Bit::getAux(UInt8 idx, unsigned int /*palIdx*/, bool rgba)
{
    prepareAuxTexture(idx - 1, tileTmp);
    if (rgba) {
        masterRGB_->apply(4096, tileTmp.data(), tileTmpRGBA.data(), true);
        return tileTmpRGBA;
    }
    masterRGB_->apply(4096, tileTmp.data(), tileTmpRGB.data(), false);
    return tileTmpRGB;
}

/* RGBPalette */
Graphics8Bit::RGBPalette::RGBPalette() = default;

Graphics8Bit::RGBPalette::RGBPalette(const std::string &palette)
{
    Util::PhysFSFile styleFile { palette };
    loadFromFile(styleFile);
}

Graphics8Bit::RGBPalette::RGBPalette(Util::PhysFSFile &styleFile)
{
    loadFromFile(styleFile);
}

int Graphics8Bit::RGBPalette::loadFromFile(Util::PhysFSFile &styleFile)
{
    styleFile.read(data, sizeof(data));
    return 0;
}

void Graphics8Bit::RGBPalette::apply(unsigned int len, const unsigned char *src, unsigned char *dst, bool rgba)
{
    for (unsigned int i = 0; i < len; i++) {
        const auto tmp = *src * 3;
        for (int j = 0; j < 3; j++) {
            *dst = data[tmp + j];
            ++dst;
        }
        if (rgba) {
            if (*src == 0)
                *dst = 0x00;
            else
                *dst = 0xff;
            ++dst;
        }
        ++src;
    }
}
} // namespace OpenGTA
