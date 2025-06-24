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
#ifndef GL_CITYVIEW_H
#define GL_CITYVIEW_H

#include <array>
#include <span>

#include <SDL2/SDL.h>
#include <core/map.h>
#include <core/navdata.h>

#include <graphics/frustum.h>
#include <graphics/pagedtexture.h>
#include <graphics/texturecache.h>

namespace OpenGL {
class Camera;
class Screen;
} // namespace OpenGL

namespace OpenGTA {

class BlockAnimCtrl;
class GraphicsBase;
class CityView {
public:
    CityView(OpenGL::Screen &screen, OpenGL::Camera &camera);
    ~CityView();
    void loadMap(const std::string &map, const std::string &style);
    static void createLevelObject(OpenGTA::Map::ObjectPosition *obj);
    void setPosition(const GLfloat &x, const GLfloat &y, const GLfloat &z);
    void setTopDownView(const GLfloat &height);
    // void setCamVector(const GLfloat & x, const GLfloat & y, const GLfloat & z);
    void setZoom(GLfloat zoom);
    void setViewMode(bool topDown);
    [[nodiscard]] bool getViewMode() const noexcept { return topDownView; }
    void setDrawHeadingArrows(bool yes) noexcept { drawHeadingMarkers = yes; }
    void setTexFlipTest(int v) noexcept { texFlipTest = v; }
    [[nodiscard]] std::span<const GLfloat> getCamPos() const noexcept { return camPos; }
    void setVisibleRange(int);
    [[nodiscard]] int getVisibleRange() const;
    void getTerrainHeight(GLfloat &x, GLfloat &y, GLfloat &z);
    void draw(Uint32 ticks);
    NavData::Sector *getCurrentSector() noexcept { return current_sector; }
    OpenGL::PagedTexture renderMap2Texture();

    [[nodiscard]] bool getDrawTextured() const;
    [[nodiscard]] bool getDrawLines() const;
    [[nodiscard]] bool getDrawLinesBlockColor() const;
    void setDrawTextured(bool v);
    void setDrawLines(bool v);
    void setDrawLinesBlockColor(bool v);

    void resetTextures();
    [[nodiscard]] const SDL_Rect &getActiveRect() const noexcept { return activeRect; }
    [[nodiscard]] const SDL_Rect &getOnScreenRect() const noexcept { return drawnRect; }
    BlockAnimCtrl *blockAnims {};

protected:
    void setNull();
    void cleanup();
    void drawBlock(OpenGTA::Map::BlockInfo *bi);
    void drawObject(OpenGTA::Map::ObjectPosition *);
    // OpenGL::PagedTexture createSprite(size_t sprNum, GraphicsBase::SpriteInfo* info);
    Util::CFrustum frustum {};
    OpenGL::TextureCache<uint8_t> *sideCache {};
    OpenGL::TextureCache<uint8_t> *lidCache {};
    OpenGL::TextureCache<uint8_t> *auxCache {};
    Map *loadedMap {};
    OpenGTA::GraphicsBase *style {};
    GLfloat zoomLevel {};
    std::array<GLfloat, 3> camPos {};
    std::array<GLfloat, 3> camVec {};
    int visibleRange {};
    bool topDownView {};
    bool drawTextured {};
    bool drawLines {};
    bool drawLinesBlockType {};
    bool drawHeadingMarkers {};
    uint8_t aboveBlockType {};

    SDL_Rect activeRect {};
    SDL_Rect drawnRect {};

    int scene_rendered_vertices {};
    int scene_rendered_blocks {};

    GLuint scene_display_list {};
    bool scene_is_dirty {};
    int texFlipTest {};

    Uint32 lastCacheEmptyTicks {};

    NavData::Sector *current_sector {};

    OpenGL::Screen &screen_;
    OpenGL::Camera &camera_;
};
} // namespace OpenGTA

#endif
