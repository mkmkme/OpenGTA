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

struct SpriteIdentifier;
struct SpriteIdentifier {
    UInt16 sprNum = 0;
    Int16 remap = -1;
    UInt32 delta = 0;
    SpriteIdentifier() = default;
    SpriteIdentifier(UInt16, Int16, UInt32);
    SpriteIdentifier(const SpriteIdentifier &other);
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
    bool has(UInt16 sprNum);
    bool has(UInt16 sprNum, Int16 remap);
    bool has(const SpriteIdentifier &si);
    PagedTexture &get(UInt16 sprNum);
    PagedTexture &get(UInt16 sprNum, Int16 remap);
    PagedTexture &get(const SpriteIdentifier &si);
    void add(UInt16 sprNum, PagedTexture &t);
    void add(UInt16 sprNum, Int16 remap, PagedTexture &t);
    void add(const SpriteIdentifier &si, PagedTexture &t);
    PagedTexture create(
        UInt16 sprNum,
        OpenGTA::GraphicsBase::SpriteNumbers::SpriteTypes,
        Int16 remap
    );
    PagedTexture create(
        UInt16 sprNum,
        OpenGTA::GraphicsBase::SpriteNumbers::SpriteTypes,
        Int16 remap,
        UInt32 delta
    );

    [[nodiscard]] OpenGL::PagedTexture createSprite(
        size_t sprite_num,
        Int16 remap,
        UInt32 delta,
        const OpenGTA::SpriteInfo &info
    ) const;

private:
    using SpriteMapType = std::map<SpriteIdentifier, PagedTexture>;
    SpriteMapType loadedSprites;
    bool doScale2x;
};
} // namespace OpenGL

#endif
