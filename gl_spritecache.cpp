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
#include <map>
#include <memory>
#include <cassert>
#include "config.h"
#include "gl_spritecache.h"
#include "image_loader.h"
#include "dataholder.h"
#include "log.h"
#include "sprite-info.h"

namespace OpenGL {
  SpriteIdentifier::SpriteIdentifier(PHYSFS_uint16 num, PHYSFS_sint16 map, PHYSFS_uint32 d) :
    sprNum(num), remap(map), delta(d) {}
  SpriteIdentifier::SpriteIdentifier(const SpriteIdentifier & other) = default;

  SpriteCache::SpriteCache() {
#ifdef DO_SCALE2X
    doScale2x = true;
#else
    doScale2x = false;
#endif
  }

  void SpriteCache::setScale2x(bool enabled) {
#ifndef DO_SCALE2X
    if (enabled)
      // FIXME: for some reason I can not catch this exception, thus it only prints
      //throw E_NOTSUPPORTED("Scale2x feature disabled at compile time");
      ERROR("scale2x feature disabled at compile time - ignoring request");
#endif
    if (loadedSprites.begin() == loadedSprites.end()) {
      doScale2x = enabled;
    }
    else {
      ERROR("scale2x cannot be set during game - ignoring request");
    }
  }

  bool SpriteCache::getScale2x() const {
    return doScale2x;
  }

  SpriteCache::~SpriteCache() {
    clearAll();
  }

  void SpriteCache::clearAll() {
    for (auto &sprite : loadedSprites) {
      glDeleteTextures(1, &sprite.second.inPage);
    }
    loadedSprites.clear();
  }

  bool SpriteCache::has(PHYSFS_uint16 sprNum) {
    auto i = loadedSprites.find(SpriteIdentifier(sprNum, -1, 0));
    if (i != loadedSprites.end())
      return true;
    INFO("sprite not loaded sprnum: {}", sprNum);
    return false;
  }

  bool SpriteCache::has(PHYSFS_uint16 sprNum, PHYSFS_sint16 remap) {
    auto i = loadedSprites.find(SpriteIdentifier(sprNum, remap, 0));
    if (i != loadedSprites.end())
      return true;
    INFO("sprite not loaded sprnum: {} remap: {}", sprNum, remap);
    return false;
  }

  bool SpriteCache::has(const SpriteIdentifier & si) {
    auto i = loadedSprites.find(si);
    if (i != loadedSprites.end())
      return true;
    INFO("sprite not loaded sprnum: {} remap: {} delta: {}",
         si.sprNum,
         si.remap,
         si.delta);
    return false;
  }

  PagedTexture & SpriteCache::get(PHYSFS_uint16 sprNum) {
    auto i = loadedSprites.find(SpriteIdentifier(sprNum, -1, 0));
    assert(i != loadedSprites.end());
    return i->second;
  }

  PagedTexture & SpriteCache::get(PHYSFS_uint16 sprNum, PHYSFS_sint16 remap) {
    auto i = loadedSprites.find(SpriteIdentifier(sprNum, remap, 0));
    assert(i != loadedSprites.end());
    return i->second;
  }

  PagedTexture & SpriteCache::get(const SpriteIdentifier & si) {
    auto i = loadedSprites.find(si);
    assert(i != loadedSprites.end());
    return i->second;
  }

  void SpriteCache::add(PHYSFS_uint16 sprNum, PHYSFS_sint16 remap, PagedTexture & t) {
    loadedSprites.insert(
      std::make_pair(
        SpriteIdentifier(sprNum, remap, 0), t));
  }

  void SpriteCache::add(const SpriteIdentifier & si, PagedTexture & t) {
    loadedSprites.insert(std::make_pair(si, t));
  }

  PagedTexture SpriteCache::create(PHYSFS_uint16 sprNum, 
    OpenGTA::GraphicsBase::SpriteNumbers::SpriteTypes st, PHYSFS_sint16 remap  = -1 ) {
    /*
    OpenGTA::GraphicsBase & style = OpenGTA::ActiveStyle::Instance().get();
    PHYSFS_uint16 real_num = style.spriteNumbers.reIndex(sprNum, st);

    OpenGTA::GraphicsBase::SpriteInfo* info = style.getSprite(real_num);
    assert(info);
    
    OpenGL::PagedTexture t = createSprite(real_num, remap, info);
    add(real_num, remap, t);
    return t;
    */
    return create(sprNum, st, remap, 0);
  }

  PagedTexture SpriteCache::create(PHYSFS_uint16 sprNum,
        OpenGTA::GraphicsBase::SpriteNumbers::SpriteTypes st,
        PHYSFS_sint16 remap, PHYSFS_uint32 delta) {
    OpenGTA::GraphicsBase & style = OpenGTA::ActiveStyle::Instance().get();
    PHYSFS_uint16 real_num = style.spriteNumbers.reIndex(sprNum, st);

    OpenGTA::SpriteInfo* info = style.getSprite(real_num);
    assert(info);
    
    OpenGL::PagedTexture t = createSprite(real_num, remap, delta, info);
    SpriteIdentifier si(real_num, remap, delta);
    add(si, t);
    return t;
  }

  OpenGL::PagedTexture SpriteCache::createSprite(size_t sprite_num, PHYSFS_sint16 remap,
    PHYSFS_uint32 delta, OpenGTA::SpriteInfo* info) const {
    INFO("creating new sprite: {} remap: {}", sprite_num, remap);
    auto src_smart = OpenGTA::ActiveStyle::Instance().get().getSpriteBitmap(
      sprite_num, remap , delta
    );
    unsigned char* src = src_smart.get();
    #if 0
    if (sprite_num == 257) {
      info->w = 72;
      info->h = 72;
      SDL_RWops * rwops = PHYSFSRWOPS_openRead("tree.png");//file.c_str());
      SDL_Surface *surface = IMG_Load_RW(rwops, 1);
      assert(surface);
      uint16_t bpp = surface->format->BytesPerPixel;
      ImageUtil::NextPowerOfTwo npot(surface->w, surface->h);
      uint8_t * buffer = Util::BufferCache::Instance().requestBuffer(npot.w * npot.h * bpp);
      SDL_LockSurface(surface);
      ImageUtil::copyImage2Image(buffer, (uint8_t*)surface->pixels, surface->pitch, surface->h,
          npot.w * bpp);
      SDL_UnlockSurface(surface);

      GLuint texture = ImageUtil::createGLTexture(npot.w, npot.h, (bpp == 4) ? true : false, buffer);
      return OpenGL::PagedTexture(texture, 0, 0, GLfloat(surface->w)/npot.w, GLfloat(surface->h)/npot.h);

    }
    #endif
#if 0
    int glwidth = 1;
    int glheight = 1;

    while(glwidth < info->w)
      glwidth <<= 1;
    while(glheight < info->h)
      glheight <<= 1;
    unsigned char* dst = Util::BufferCache::Instance().requestBuffer(glwidth * glheight * 4);
    assert(dst != NULL);
    unsigned char * t = dst;
    unsigned char * r = src;
    for (unsigned int i = 0; i < info->h; i++) {
      memcpy(t, r, info->w * 4);
      t += glwidth * 4;
      r += info->w * 4;
    }
#endif
    ImageUtil::NextPowerOfTwo npot(info->w, info->h);
    auto dst = std::make_unique<uint8_t[]>(npot.w * npot.h * 4);

    ImageUtil::copyImage2Image(dst.get(), src, info->w * 4, info->h, npot.w * 4);

#ifdef DO_SCALE2X
    if (doScale2x) {
      dst = ImageUtil::scale2x_32bit(dst.get(), npot.w, npot.h);
    }
#endif

    GLuint texid = (doScale2x) ?
        ImageUtil::createGLTexture(npot.w * 2, npot.h * 2, true, dst.get())
      : ImageUtil::createGLTexture(npot.w, npot.h, true, dst.get());
#if 0
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifdef DO_SCALE2X
    if (doScale2x)
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glwidth*2, glheight*2, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);
    else
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glwidth, glheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glwidth, glheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst);
#endif

#endif
    return {texid, 0, 0,
      float(info->w)/float(npot.w), float(info->h)/float(npot.h)};
  }

}
