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

#include "gl_font.h"

#include <map>
#include <memory>
#include <string>

namespace OpenGTA {

class FontCache {
public:
    FontCache(const FontCache &copy) = delete;
    FontCache &operator=(const FontCache &copy) = delete;

    OpenGL::DrawableFont &getFont(const std::string &file, uint32_t scale);

    static FontCache &Instance()
    {
        static FontCache instance;
        return instance;
    }

private:
    FontCache() = default;
    struct FontIdentifier {
        const std::string filename;
        const uint32_t scale;
        auto operator<=>(const FontIdentifier &) const = default;
    };
    std::map<FontIdentifier, std::unique_ptr<OpenGL::DrawableFont>> loaded_fonts_;
};
} // namespace OpenGTA
