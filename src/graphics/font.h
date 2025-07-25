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
#pragma once

#include <memory>
#include <string>

#include "graphics/base.h"
#include "graphics/texturecache.h"

#include "core/font.h"

namespace OpenGL {
class DrawableFont {
public:
    DrawableFont(const std::string &filename, unsigned int scale) noexcept;
    DrawableFont(const DrawableFont &) = delete;
    DrawableFont(DrawableFont &&) noexcept = default;
    GLfloat drawString(const std::string &text);
    GLfloat drawString_r2l(const std::string &text);
    [[nodiscard]] uint16_t getHeight() const noexcept { return scale * fontSource->getCharHeight(); }
    void resetTextures() noexcept;

private:
    void cleanup() noexcept;
    void clearCached() noexcept;
    FontQuad createDrawableCharacter(char c);
    std::unique_ptr<OpenGTA::Font> fontSource;
    std::string srcName;
    std::unique_ptr<TextureCache<char>> texCache;
    std::map<char, FontQuad> drawables;
    unsigned int scale;
};
} // namespace OpenGL
