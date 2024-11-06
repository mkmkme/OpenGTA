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
#include <cassert>
#include <cstring>
#include <memory>

#include <core/font.h>

#include <graphics/font.h>
#include <util/errors.h>

#include "base.h"

namespace OpenGL {
DrawableFont::DrawableFont()
{
    fontSource = nullptr;
    texCache = nullptr;
    scale = 1;
}
DrawableFont::~DrawableFont()
{
    cleanup();
}
void DrawableFont::setScale(unsigned int newScale)
{
    scale = newScale;
    clearCached();
}
void DrawableFont::clearCached()
{
    drawables.clear();
}
void DrawableFont::resetTextures()
{
    clearCached();
    texCache->clearAll();
}
void DrawableFont::loadFont(const std::string &filename)
{
    cleanup();
    fontSource = std::make_unique<OpenGTA::Font>(filename);
    texCache = std::make_unique<TextureCache<char>>(("FontTextures: " + filename).c_str());
    srcName.clear();
    srcName = filename;
}
void DrawableFont::cleanup()
{
    clearCached();
    fontSource.reset();
    texCache.reset();
}
GLfloat DrawableFont::drawString(const std::string &text)
{
    assert(texCache != nullptr);
    assert(fontSource != nullptr);
    GLfloat move = 0.0f;
    for (char c : text) {
        if (c != ' ') {
            auto j = drawables.find(c);
            if (j == drawables.end()) {
                j = drawables.emplace(c, createDrawableCharacter(c)).first;
            }
            Renderer<FontQuad>::draw(j->second);
        }
        GLfloat mm = float(fontSource->getMoveWidth(c)) * 1.1f * scale;
        glTranslatef(mm, 0.0f, 0.0f);
        move += mm;
    }
    return move;
}

GLfloat DrawableFont::drawString_r2l(const std::string &text)
{
    assert(texCache != nullptr);
    assert(fontSource != nullptr);
    GLfloat move = 0.0f;
    for (char c : text) {
        if (c != ' ') {
            auto j = drawables.find(c);
            if (j == drawables.end()) {
                j = drawables.emplace(c, createDrawableCharacter(c)).first;
            } else {
                GLfloat mm = float(fontSource->getMoveWidth(c)) * 1.1f * scale;
                glTranslatef(-mm, 0.0f, 0.0f);
                move += mm;
            }
            Renderer<FontQuad>::draw(j->second);
        }
    }
    return move;
}

uint16_t DrawableFont::getHeight()
{
    return scale * fontSource->getCharHeight();
}

FontQuad DrawableFont::createDrawableCharacter(char c)
{
    GLuint texid;
    unsigned int w;
    unsigned int h;
    const auto src = fontSource->getCharacterBitmap(fontSource->getIdByChar(c), &w, &h);
    unsigned int glwidth = 1;
    unsigned int glheight = 1;

    while (glwidth < w)
        glwidth <<= 1;

    while (glheight < h)
        glheight <<= 1;

    std::vector<UInt8> dst(glwidth * glheight * 4);
    unsigned char *t = dst.data();
    const unsigned char *r = src.data();
    for (unsigned int i = 0; i < h; i++) {
        memcpy(t, r, w * 4);
        t += glwidth * 4;
        r += w * 4;
    }
    glGenTextures(1, &texid);
    glBindTexture(GL_TEXTURE_2D, texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glwidth, glheight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dst.data());
    texCache->addTexture(c, texid);

    FontQuad res;
    res.vertices[0][0] = res.vertices[0][1] = 0;
    res.vertices[1][0] = w * scale;
    res.vertices[1][1] = 0;
    res.vertices[2][0] = w * scale;
    res.vertices[2][1] = h * scale;
    res.vertices[3][0] = 0;
    res.vertices[3][1] = h * scale;

    float glw = float(w) / float(glwidth);
    float glh = float(h) / float(glheight);
    res.texCoords[0][0] = 0.0f;
    res.texCoords[0][1] = glh;
    res.texCoords[1][0] = glw;
    res.texCoords[1][1] = glh;
    res.texCoords[2][0] = glw;
    res.texCoords[2][1] = 0.0f;
    res.texCoords[3][0] = 0.0f;
    res.texCoords[3][1] = 0.0f;

    res.texId = texid;

    return res;
}
} // namespace OpenGL
