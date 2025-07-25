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
#ifndef SPRITE_CACHE_H
#define SPRITE_CACHE_H
#include <cstddef>

#include <physfs.h>

#include <core/graphics-base.h>

#include <graphics/pagedtexture.h>
#include <graphics/texturecache.h>

namespace OpenGTA {
class SpriteInfo;
}

namespace OpenGL {

struct SpriteIdentifier {
    uint16_t sprNum = 0;
    int16_t remap = -1;
    uint32_t delta = 0;
    SpriteIdentifier() = default;
    SpriteIdentifier(uint16_t, int16_t, uint32_t);
    auto operator<=>(const SpriteIdentifier &) const = default;
};

class SpriteCache {
private:
    SpriteCache();
    ~SpriteCache();

public:
    SpriteCache(const SpriteCache &copy) = delete;
    SpriteCache &operator=(const SpriteCache &copy) = delete;

    static SpriteCache &Instance()
    {
        static SpriteCache instance;
        return instance;
    }

    void clearAll();
    [[nodiscard]] bool getScale2x() const;
    void setScale2x(bool enabled);
    bool has(uint16_t sprNum);
    bool has(uint16_t sprNum, int16_t remap);
    bool has(const SpriteIdentifier &si);
    PagedTexture &get(uint16_t sprNum);
    PagedTexture &get(uint16_t sprNum, int16_t remap);
    PagedTexture &get(const SpriteIdentifier &si);
    void add(uint16_t sprNum, PagedTexture &t);
    void add(uint16_t sprNum, int16_t remap, PagedTexture &t);
    void add(const SpriteIdentifier &si, PagedTexture &t);
    PagedTexture create(uint16_t sprNum, OpenGTA::GraphicsBase::SpriteNumbers::SpriteType, int16_t remap);
    PagedTexture
    create(uint16_t sprNum, OpenGTA::GraphicsBase::SpriteNumbers::SpriteType, int16_t remap, uint32_t delta);

    [[nodiscard]] OpenGL::PagedTexture
    createSprite(size_t sprite_num, int16_t remap, uint32_t delta, const OpenGTA::SpriteInfo &info) const;

private:
    using SpriteMapType = std::map<SpriteIdentifier, PagedTexture>;
    SpriteMapType loadedSprites;
    bool doScale2x;
};
} // namespace OpenGL

#endif
