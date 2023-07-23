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
#include "graphics-24bit.h"

#include <iostream>
#include <cassert>
#include "log.h"
#include "sprite-info.h"

using namespace Util;
namespace OpenGTA {

#define GTA_GRAPHICS_GRY 325
#define GTA_GRAPHICS_G24 336

  Graphics24Bit::Graphics24Bit(const std::string& style) : GraphicsBase() {
    fd = PHYSFS_openRead(style.c_str());
    assert(fd != nullptr);
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
    }
    else if (style.find("002") != std::string::npos) {
      firstValidPedRemap = 79;
      lastValidPedRemap = 135;
    }
    else if (style.find("003") != std::string::npos) {
      // already set
    }
    else {
      WARN("Unknown g24 style - ped remaps most likely broken!");
    }
  }

  Graphics24Bit::~Graphics24Bit() {
    delete [] rawClut;
    delete [] palIndex;
    PHYSFS_close(fd);
  }

  void Graphics24Bit::loadHeader() {
    PHYSFS_uint32 vc;
    PHYSFS_readULE32(fd, &vc);
    if(vc != GTA_GRAPHICS_G24) {
      ERROR("graphics file specifies version {} (should be {})",
            vc,
            GTA_GRAPHICS_G24);
      return;
    }
    PHYSFS_readULE32(fd, &sideSize);
    PHYSFS_readULE32(fd, &lidSize);
    PHYSFS_readULE32(fd, &auxSize);
    PHYSFS_readULE32(fd, &animSize);
    PHYSFS_readULE32(fd, &clutSize);
    PHYSFS_readULE32(fd, &tileclutSize);
    PHYSFS_readULE32(fd, &spriteclutSize);
    PHYSFS_readULE32(fd, &newcarclutSize);
    PHYSFS_readULE32(fd, &fontclutSize);
    PHYSFS_readULE32(fd, &paletteIndexSize);
    PHYSFS_readULE32(fd, &objectInfoSize);
    PHYSFS_readULE32(fd, &carInfoSize);
    PHYSFS_readULE32(fd, &spriteInfoSize);
    PHYSFS_readULE32(fd, &spriteGraphicsSize);
    PHYSFS_readULE32(fd, &spriteNumberSize);

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
    
    PHYSFS_uint32 tmp = sideSize / 4096 + lidSize / 4096 + auxSize / 4096;
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
        spriteNumberSize);
    if (spriteNumberSize != 42) {
      ERROR("spriteNumberSize is {} (should be 42)", spriteNumberSize);
      return;
    }

    INFO("clut: {} tileclut: {} spriteclut: {} newcar: {} fontclut: {}",
         clutSize,
         tileclutSize,
         spriteclutSize,
         newcarclutSize,
         fontclutSize);
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

  void Graphics24Bit::loadClut() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize;
    PHYSFS_seek(fd, st);
    pagedClutSize = clutSize;
    if (clutSize % 65536 != 0)
      pagedClutSize += (65536 - (clutSize % 65536));
    rawClut = new unsigned char[pagedClutSize];
    assert(rawClut);
    PHYSFS_readBytes(fd, rawClut, pagedClutSize);
  }

  void Graphics24Bit::loadPalIndex() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
      pagedClutSize;
    PHYSFS_seek(fd, st);
    PHYSFS_uint16 pal_index_count = paletteIndexSize / 2;
    assert(paletteIndexSize % 2 == 0);
    palIndex = new PHYSFS_uint16[pal_index_count];
    for (PHYSFS_uint16 i = 0; i < pal_index_count; i++) {
      PHYSFS_readULE16(fd, &palIndex[i]);
    }
  }

  void Graphics24Bit::loadCarInfo() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
      pagedClutSize + paletteIndexSize + objectInfoSize;
    // INFO("seek for {}", st);
    loadCarInfo_shared(st);
  }

  void Graphics24Bit::loadSpriteInfo() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
      pagedClutSize + paletteIndexSize + objectInfoSize + 
      carInfoSize; 
    PHYSFS_seek(fd, st);

    PHYSFS_uint8 v;
    PHYSFS_uint32 w;
    PHYSFS_uint32 _bytes_read = 0;
    while (_bytes_read < spriteInfoSize) {
      auto *si = new SpriteInfo();
      PHYSFS_readBytes(fd, static_cast<void*>(&si->w), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&si->h), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&si->deltaCount), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&v), 1);
      PHYSFS_readULE16(fd, &si->size);
      _bytes_read += 6;
      PHYSFS_readULE16(fd, &si->clut);
      PHYSFS_readBytes(fd, static_cast<void*>(&si->xoffset), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&si->yoffset), 1);
      PHYSFS_readULE16(fd, &si->page);
      _bytes_read += 6;
      /*
      std::cout << "sprite: " << int(si->w) << "x" << int(si->h) << " deltas: " << int(si->deltaCount)
      << " clut: " << si->clut << " x: " << int(si->xoffset) << " y: " << int(si->yoffset) <<
      " page: " << si->page << std::endl;
      */
      // sanity check
      if (v)
        WARN("Compression flag active in sprite!");
      if (int(si->w) * int(si->h) != int(si->size)) {
        ERROR("Sprite info size mismatch: {}x{} != {}",
              int(si->w),
              int(si->h),
              si->size);
        return;
      }
      if (si->deltaCount > 32) {
        ERROR("Delta count of sprite is {}", si->deltaCount);
        return;
      }
      for (PHYSFS_uint8 j = 0; j < si->deltaCount; j++) {
        si->delta[j].size = 0;
        si->delta[j].ptr = nullptr;
        if (si->deltaCount && (j < si->deltaCount)) {
          PHYSFS_readULE16(fd, &si->delta[j].size);
          PHYSFS_readULE32(fd, &w);
          _bytes_read += 6;
          si->delta[j].ptr = reinterpret_cast<unsigned char*>(w);
        }
      }
      spriteInfos.push_back(si);
    }
    st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
      pagedClutSize + paletteIndexSize + objectInfoSize + 
      carInfoSize + spriteInfoSize;
    assert(PHYSFS_tell(fd) == PHYSFS_sint64(st));

  }

  void Graphics24Bit::loadSpriteNumbers() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
      pagedClutSize + paletteIndexSize + objectInfoSize + 
      carInfoSize + spriteInfoSize + spriteGraphicsSize;
    loadSpriteNumbers_shared(st);
  }

  void Graphics24Bit::loadSpriteGraphics() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
      pagedClutSize + paletteIndexSize + objectInfoSize + 
      carInfoSize + spriteInfoSize;
    PHYSFS_seek(fd, st);

    rawSprites = new unsigned char[spriteGraphicsSize];
    PHYSFS_readBytes(fd, static_cast<void*>(rawSprites), spriteGraphicsSize);

    auto i = spriteInfos.cbegin();
    auto end = spriteInfos.cend();
    PHYSFS_uint32 _pagewise = 256 * 256;
    while (i != end) {
      SpriteInfo *info = *i;
      for (uint8_t k = 0; k < info->deltaCount; ++k) {
        const auto offset = reinterpret_cast<uintptr_t>(info->delta[k].ptr);
        const auto page = offset / 65536;
        const auto y = (offset % 65536) / 256;
        const auto x = (offset % 65536) % 256;
        info->delta[k].ptr = rawSprites + page * _pagewise + 256 * y + x;
      }
      i++;
    }
  }

  void Graphics24Bit::loadObjectInfo() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize +
      pagedClutSize + paletteIndexSize;
    loadObjectInfo_shared(st);
  }

  void Graphics24Bit::applyClut(unsigned char* src, unsigned char* dst, 
      const size_t & len, const PHYSFS_uint16 & clutIdx, bool rgba) {
    PHYSFS_uint32 off = 65536 * (clutIdx / 64) + 4 * (clutIdx % 64);
    for (size_t i= 0; i < len; i++) {
      PHYSFS_uint32 coff = PHYSFS_uint32(*src) * 256 + off;
      *dst = rawClut[coff+2];
      ++dst;
      *dst = rawClut[coff+1];
      ++dst;
      *dst = rawClut[coff+0];
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

  unsigned char *Graphics24Bit::getLid(unsigned int idx, unsigned int _not_used, bool rgba) {
    prepareLidTexture(idx - 1, reinterpret_cast<unsigned char*>(tileTmp));
    unsigned char* src = tileTmp;
    unsigned char* dst = (rgba) ? tileTmpRGBA : tileTmpRGB;
    PHYSFS_uint16 clutIdx = palIndex[4 * (idx + sideSize / 4096)];
    applyClut(src, dst, 4096, clutIdx, rgba);

    return (rgba) ? tileTmpRGBA : tileTmpRGB;
  }
  
  unsigned char *Graphics24Bit::getSide(unsigned int idx, unsigned int _not_used, bool rgba) {
    prepareSideTexture(idx-1, reinterpret_cast<unsigned char*>(tileTmp));
    unsigned char* src = tileTmp;
    unsigned char* dst = (rgba) ? tileTmpRGBA : tileTmpRGB;
    PHYSFS_uint16 clutIdx = palIndex[idx*4];
    applyClut(src, dst, 4096, clutIdx, rgba);

    return (rgba) ? tileTmpRGBA : tileTmpRGB;
  }
  
  unsigned char *Graphics24Bit::getAux(unsigned int idx, unsigned int _not_used, bool rgba) {
    prepareAuxTexture(idx - 1, reinterpret_cast<unsigned char*>(tileTmp));
    
    unsigned char* src = tileTmp;
    unsigned char* dst = (rgba) ? tileTmpRGBA : tileTmpRGB;
    PHYSFS_uint16 clutIdx = palIndex[4 * (idx + sideSize / 4096 + lidSize / 4096)];
    applyClut(src, dst, 4096, clutIdx, rgba);

    return (rgba) ? tileTmpRGBA : tileTmpRGB;
  }

  std::unique_ptr<unsigned char[]> Graphics24Bit::getSpriteBitmap(size_t id, int remap, uint32_t delta) {
    const SpriteInfo *info = spriteInfos[id];
    assert(info != nullptr);
    const PHYSFS_uint32 y = info->yoffset;
    const PHYSFS_uint32 x = info->xoffset;
    const PHYSFS_uint32 page_size = 256 * 256;

    unsigned char * page_start = rawSprites + info->page * page_size;
    
    auto dest = std::make_unique<unsigned char[]>(page_size);
    memcpy(dest.get(), page_start, page_size);
    if (delta > 0) {
      handleDeltas(*info, dest.get(), delta);
      /*
      assert(delta < info->deltaCount);
      DeltaInfo & di = info->delta[delta];
      applyDelta(*info, dest+256*y+x, di);
      */
    }

    auto bigbuf_smart = std::make_unique<unsigned char[]>(page_size * 4);
    unsigned char* bigbuf = bigbuf_smart.get();
    unsigned char* result = dest.get();
    unsigned int skip_cluts = 0;
    if (remap > -1)
      skip_cluts = spriteclutSize / 1024 + remap + 1;

    PHYSFS_uint16 clutIdx = palIndex[info->clut + tileclutSize / 1024] + skip_cluts;
    //  PHYSFS_uint16 clutIdx = palIndex[info->clut + (spriteclutSize + tileclutSize) / 1024] + (remap > -1 ? remap+2 : 0);
    applyClut(dest.get(), bigbuf, page_size, clutIdx, true);
    assert(page_size > PHYSFS_uint32(info->w * info->h * 4));
    for (uint16_t i = 0; i < info->h; i++) {
      memcpy(result, bigbuf+(256*y+x)*4, info->w * 4);
      result += info->w * 4;
      bigbuf += 256 * 4;
    }

    return dest;
  }
}
