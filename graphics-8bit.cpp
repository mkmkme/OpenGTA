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
#include "graphics-8bit.h"

#include <iostream>
#include <cassert>
#include "car-info.h"
#include "file_helper.h"
#include "loaded-anim.h"
#include "log.h"
#include "m_exceptions.h"
#include "object-info.h"
#include "sprite-info.h"
#include "set.h"

using namespace Util;

namespace OpenGTA {

#define GTA_GRAPHICS_GRX 290
#define GTA_GRAPHICS_GRY 325
#define GTA_GRAPHICS_G24 336

  Graphics8Bit::Graphics8Bit(const std::string& style) : GraphicsBase() {
    fd = Util::FileHelper::OpenReadVFS(style);
    _topHeaderSize = 52;
    rawTiles = NULL;
    rawSprites = NULL;
    auxBlockTrailSize = 0;
    loadHeader();
    setupBlocking(style);
    firstValidPedRemap = 131;
    lastValidPedRemap = 187;
  }

  void Graphics8Bit::dump() {
    
    uint32_t gs = sideSize + lidSize + auxSize;

    INFO("* graphics info *");
    INFO("{} bytes in {} pages {} images", gs, gs / 65536, gs / 4096);
    INFO("{} sprites ({}) total: {} bytes",
         spriteInfos.size(),
         spriteInfoSize,
         spriteGraphicsSize);
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

  void Graphics8Bit::loadHeader() {
    PHYSFS_uint32 vc;
    PHYSFS_readULE32(fd, &vc);
    if(vc != GTA_GRAPHICS_GRY) {
        ERROR("graphics file specifies version {} instead of {}",
              vc,
              GTA_GRAPHICS_GRY);
        throw E_INVALIDFORMAT("8-bit loader failed");
        return;
    }
    PHYSFS_readULE32(fd, &sideSize);
    PHYSFS_readULE32(fd, &lidSize);
    PHYSFS_readULE32(fd, &auxSize);
    PHYSFS_readULE32(fd, &animSize);
    PHYSFS_readULE32(fd, &paletteSize);
    PHYSFS_readULE32(fd, &remapSize);
    PHYSFS_readULE32(fd, &remapIndexSize);
    PHYSFS_readULE32(fd, &objectInfoSize);
    PHYSFS_readULE32(fd, &carInfoSize);
    PHYSFS_readULE32(fd, &spriteInfoSize);
    PHYSFS_readULE32(fd, &spriteGraphicsSize);
    PHYSFS_readULE32(fd, &spriteNumberSize);
    
    INFO("Block textures: S {} L {} A {}",
         sideSize / 4096,
         lidSize / 4096,
         auxSize / 4096);
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
    INFO("Anim size: {} palette size: {} remap size: {} remap-index size: {}",
         animSize,
         paletteSize,
         remapSize,
         remapIndexSize);
    INFO(
        "Obj-info size: {} car-size: {} sprite-info size: {} graphic size: {} "
        "numbers s: {}",
        objectInfoSize,
        carInfoSize,
        spriteInfoSize,
        spriteGraphicsSize,
        spriteNumberSize);
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


  
  void Graphics8Bit::loadPalette() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize;
    PHYSFS_seek(fd, st);
    masterRGB_.reset(new RGBPalette(fd));
  }
  
  void Graphics8Bit::loadRemapTables() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize;
    PHYSFS_seek(fd, st);
    PHYSFS_readBytes(fd, static_cast<void*>(remapTables), sizeof(remapTables));
    /*
    for (int i=0; i < 256; i++) {
      for (int j = 0; j < 256; j++) {
        std::cout << int(remapTables[i][j]) << " ";
      }
      std::cout << std::endl;
    }*/
  }

  void Graphics8Bit::loadRemapIndex() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize +
      remapSize;
    PHYSFS_seek(fd, st);
    PHYSFS_readBytes(fd, static_cast<void*>(remapIndex), sizeof(remapIndex));
    /*
    std::cout << "LID remap tables" << std::endl;
    for (int i=0; i<256; ++i) {
      std::cout << i << ": " << int(remapIndex[i][0]) << ", " << int(remapIndex[i][1]) <<
        ", " << int(remapIndex[i][2]) << ", " << int(remapIndex[i][3]) << std::endl;
    }*/
  }

  void Graphics8Bit::loadObjectInfo() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize +
      remapSize + remapIndexSize;
    loadObjectInfo_shared(st);
  }


  void Graphics8Bit::loadCarInfo() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize +
      remapSize + remapIndexSize + objectInfoSize;
    loadCarInfo_shared(st);
  }
  
  void Graphics8Bit::loadSpriteInfo() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize +
      remapSize + remapIndexSize + objectInfoSize + carInfoSize;
    PHYSFS_seek(fd, st);

    PHYSFS_uint8 v;
    PHYSFS_uint32 w;
    PHYSFS_uint32 _bytes_read = 0;
    while (_bytes_read < spriteInfoSize) {
      SpriteInfo *si = new SpriteInfo();
      PHYSFS_readBytes(fd, static_cast<void*>(&si->w), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&si->h), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&si->deltaCount), 1);
      PHYSFS_readBytes(fd, static_cast<void*>(&v), 1);
      PHYSFS_readULE16(fd, &si->size);
      PHYSFS_readULE32(fd, &w);
      _bytes_read += 10;
      //si->ptr = reinterpret_cast<unsigned char*>(w);
      si->page = w / 65536;
      si->xoffset = (w % 65536) % 256;
      si->yoffset = (w % 65536) / 256;
      si->clut = 0;

      /*
      std::cout << int(si->w) << "," << int(si->h) << " = " << si->size << " deltas: " << int(si->deltaCount) <<
        " at " << w << std::endl;
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
        ERROR("Delta count of sprite is {} (should be <= 32)", si->deltaCount);
        return;
      }
      for (PHYSFS_uint8 j = 0; j < 33; ++j) {
        si->delta[j].size = 0;
        si->delta[j].ptr = 0;
        if (si->deltaCount && (j < si->deltaCount)) {
          //std::cout << "reading " << int(j) << std::endl;
          PHYSFS_readULE16(fd, &si->delta[j].size);
          PHYSFS_readULE32(fd, &w);
          _bytes_read += 6;
          si->delta[j].ptr = reinterpret_cast<unsigned char*>(w);
        }
      }
      spriteInfos.push_back(si);
      
    }
    st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize +
      remapSize + remapIndexSize + objectInfoSize + carInfoSize + spriteInfoSize;
    assert(PHYSFS_tell(fd) == PHYSFS_sint64(st));
  }

  void Graphics8Bit::loadSpriteGraphics() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize +
      remapSize + remapIndexSize + objectInfoSize + carInfoSize + spriteInfoSize;
    PHYSFS_seek(fd, st);
    rawSprites = new unsigned char[spriteGraphicsSize];
    assert(rawSprites != NULL);
    PHYSFS_readBytes(fd, static_cast<void*>(rawSprites), spriteGraphicsSize);

    if (spriteInfos.size() == 0) {
      INFO("No SpriteInfo post-loading work done - structure is empty");
      return;
    }
    std::vector<SpriteInfo*>::const_iterator i = spriteInfos.begin();
    std::vector<SpriteInfo*>::const_iterator end = spriteInfos.end();
    PHYSFS_uint32 _pagewise = 256 * 256;
    while (i != end) {
      SpriteInfo *info = *i;
      /*
      PHYSFS_uint32 offset = reinterpret_cast<PHYSFS_uint32>(info->ptr);
      PHYSFS_uint32 page = offset / 65536;
      PHYSFS_uint32 y = (offset % 65536) / 256;
      PHYSFS_uint32 x = (offset % 65536) % 256;
      */
      //std::cout << int(info->w) << "x" << int(info->h) << " " << int(info->deltaCount) << " deltas" << std::endl;
      //std::cout << offset << " page " << page << " x,y " << x <<","<<y<< std::endl;
//info->ptr = rawSprites + page * _pagewise + 256 * y + x;
      for (uint8_t k = 0; k < info->deltaCount; ++k) {
        const auto tmp = reinterpret_cast<uintptr_t>(info->delta[k].ptr);
        const auto offset = static_cast<PHYSFS_uint32>(tmp);
        const auto page = offset / 65536;
        const auto y = (offset % 65536) / 256;
        const auto x = (offset % 65536) % 256;
        info->delta[k].ptr = rawSprites + page * _pagewise + 256 * y + x;
      }
      i++;
    }

    return;
  }

  void Graphics8Bit::loadSpriteNumbers() {
    PHYSFS_uint64 st = static_cast<PHYSFS_uint64>(_topHeaderSize) +
      sideSize + lidSize + auxSize + auxBlockTrailSize + animSize + paletteSize +
      remapSize + remapIndexSize + objectInfoSize + carInfoSize +
      spriteInfoSize + spriteGraphicsSize;
    loadSpriteNumbers_shared(st);
  }

  std::unique_ptr<unsigned char[]> Graphics8Bit::getSpriteBitmap(
    size_t id, int remap = -1, uint32_t delta = 0
  ) {
    SpriteInfo *info = spriteInfos[id];
    assert(info != NULL);
    //PHYSFS_uint32 offset = reinterpret_cast<PHYSFS_uint32>(info->ptr);
    //const PHYSFS_uint32 page = offset / 65536;
    const PHYSFS_uint32 y = info->yoffset; // (offset % 65536) / 256;
    const PHYSFS_uint32 x = info->xoffset; // (offset % 65536) % 256;
    const PHYSFS_uint32 page_size = 256 * 256;

    unsigned char * page_start = rawSprites + info->page * page_size;// + 256 * y + x;
    assert(page_start != NULL);
    
    auto dest = std::make_unique<unsigned char[]>(page_size);

    unsigned char * result = dest.get();
    memcpy(dest.get(), page_start, page_size);
    if (delta > 0) {
      handleDeltas(*info, result, delta);
      /*
      assert(delta < info->deltaCount);
      DeltaInfo & di = info->delta[delta];
      applyDelta(*info, result+256*y+x, di);
      */
    }
    if (remap > -1)
      applyRemap(page_size, remap, result);
    auto bigbuf_smart = std::make_unique<unsigned char[]>(page_size * 4);
    auto bigbuf = bigbuf_smart.get();
    
    masterRGB_->apply(page_size, result, bigbuf, true);
    assert(page_size > PHYSFS_uint32(info->w * info->h * 4));
    for (uint16_t i = 0; i < info->h; i++) {
      memcpy(result, bigbuf+(256*y+x)*4, info->w * 4);
      result += info->w * 4;
      bigbuf += 256 * 4;
    }

    return dest;
  }

  void Graphics8Bit::applyRemap(unsigned int len, unsigned int which, unsigned char* buffer) {
    assert(buffer!=NULL);
    unsigned char* t = buffer;
    for (unsigned int i = 0; i < len; ++i) {
      *t = remapTables[which][*t]; //FIXME: is this the right order? Is this correct at all?
      t++;
    }
  }
  

  
  unsigned char* Graphics8Bit::getSide(unsigned int idx, unsigned int palIdx, bool rgba = false) {
    prepareSideTexture(idx-1, tileTmp);
    unsigned char *res;
    if (rgba) {
      masterRGB_->apply(4096, tileTmp, tileTmpRGBA, true);
      res = tileTmpRGBA;
    }
    else {
      masterRGB_->apply(4096, tileTmp, tileTmpRGB, false);
      res = tileTmpRGB;
    }
    return res;
  }

  
  unsigned char *Graphics8Bit::getLid(unsigned int idx, unsigned int palIdx, bool rgba = false) {
    prepareLidTexture(idx-1, tileTmp);
    if (palIdx > 0)
      applyRemap(4096, palIdx, tileTmp);
    
    unsigned char *res;
    if (rgba) {
      masterRGB_->apply(4096, tileTmp, tileTmpRGBA, true);
      res = tileTmpRGBA;
    }
    else {
      masterRGB_->apply(4096, tileTmp, tileTmpRGB, false);
      res = tileTmpRGB;
    }
    return res;
  }
  
  unsigned char* Graphics8Bit::getAux(unsigned int idx, unsigned int palIdx, bool rgba = false) {
    prepareAuxTexture(idx-1, tileTmp);
    unsigned char *res;
    if (rgba) {

      masterRGB_->apply(4096, tileTmp, tileTmpRGBA, true);
      res = tileTmpRGBA;
    }
    else {
      masterRGB_->apply(4096, tileTmp, tileTmpRGB, false);
      res = tileTmpRGB;
    }
    return res;
  }

  /* RGBPalette */
  Graphics8Bit::RGBPalette::RGBPalette() {
  }
  
  Graphics8Bit::RGBPalette::RGBPalette(const std::string& palette) {
    PHYSFS_file* fd = Util::FileHelper::OpenReadVFS(palette);
    loadFromFile(fd);
  }
  
  Graphics8Bit::RGBPalette::RGBPalette(PHYSFS_file* fd) {
    loadFromFile(fd);
  }
  
  int Graphics8Bit::RGBPalette::loadFromFile(PHYSFS_file* fd) {
    PHYSFS_readBytes(fd, static_cast<void*>(data), sizeof(data));
    /*
    int max_sum = 0;
    for (int i = 1; i < 256; i+=3) {
      int sum = int(data[i]) + int(data[i+1]) + int(data[i+2]);
      if (sum > max_sum)
        max_sum = sum;
    }*/
    return 0;
  }
  
  void Graphics8Bit::RGBPalette::apply(unsigned int len, const unsigned char* src,
      unsigned char* dst, bool rgba ) {
    for (unsigned int i = 0; i < len; i++) {
      *dst = data[*src * 3 ]; ++dst;
      *dst = data[*src * 3 + 1]; ++dst;
      *dst = data[*src * 3 + 2]; ++dst;
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
}
