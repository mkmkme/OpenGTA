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
#include <core/blockanim.h>
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
    CityView(const std::string &map, const std::string &style, OpenGL::Screen &screen, OpenGL::Camera &camera);
    ~CityView();
    static void createLevelObject(OpenGTA::Map::ObjectPosition *obj);
    void setPosition(GLfloat x, GLfloat y, GLfloat z);
    void setTopDownView(GLfloat height);
    // void setCamVector(const GLfloat & x, const GLfloat & y, const GLfloat & z);
    void setZoom(GLfloat zoom) { zoomLevel = zoom; }
    void setViewMode(bool topDown) { topDownView = topDown; }
    [[nodiscard]] bool getViewMode() const noexcept { return topDownView; }
    void setDrawHeadingArrows(bool yes) noexcept { drawHeadingMarkers = yes; }
    void setTexFlipTest(int v) noexcept { texFlipTest = v; }
    [[nodiscard]] std::span<const GLfloat> getCamPos() const noexcept { return camPos; }
    void setVisibleRange(int);
    [[nodiscard]] int getVisibleRange() const { return visibleRange; }
    void getTerrainHeight(GLfloat x, GLfloat &y, GLfloat z);
    void draw(uint32_t ticks);
    NavData::Sector *getCurrentSector() noexcept { return current_sector; }
    OpenGL::PagedTexture renderMap2Texture();

    [[nodiscard]] bool getDrawTextured() const { return drawTextured; }
    [[nodiscard]] bool getDrawLines() const { return drawLines; }
    [[nodiscard]] bool getDrawLinesBlockColor() const { return drawLinesBlockType; }
    void setDrawTextured(bool v) { drawTextured = v; }
    void setDrawLines(bool v) { drawLines = v; }
    void setDrawLinesBlockColor(bool v) { drawLinesBlockType = v; }

    void resetTextures();
    [[nodiscard]] const SDL_Rect &getActiveRect() const noexcept { return activeRect; }
    [[nodiscard]] const SDL_Rect &getOnScreenRect() const noexcept { return drawnRect; }
    BlockAnimCtrl &getBlockAnimCtrl() noexcept { return blockAnims; }

protected:
    void loadMap(const std::string &map, const std::string &style);
    void drawBlock(OpenGTA::Map::BlockInfo *bi);
    void drawObject(OpenGTA::Map::ObjectPosition *);
    // OpenGL::PagedTexture createSprite(size_t sprNum, GraphicsBase::SpriteInfo* info);
    Util::CFrustum frustum {};
    OpenGL::TextureCache<uint8_t> sideCache;
    OpenGL::TextureCache<uint8_t> lidCache;
    OpenGL::TextureCache<uint8_t> auxCache;
    BlockAnimCtrl blockAnims;
    Map *loadedMap { nullptr };
    OpenGTA::GraphicsBase *style {};
    GLfloat zoomLevel { 1.0f };
    std::array<GLfloat, 3> camPos {};
    std::array<GLfloat, 3> camVec { 0, 1, 0 };
    int visibleRange { 15 };
    bool topDownView { true };
    bool drawTextured { true };
    bool drawLines { false };
    bool drawLinesBlockType { true };
    bool drawHeadingMarkers { false };
    uint8_t aboveBlockType {};

    SDL_Rect activeRect {};
    SDL_Rect drawnRect {};

    int scene_rendered_vertices {};
    int scene_rendered_blocks {};

    GLuint scene_display_list { 0 };
    bool scene_is_dirty { true };
    int texFlipTest { 0 };

    uint32_t lastCacheEmptyTicks { 0 };

    NavData::Sector *current_sector { nullptr };

    OpenGL::Screen &screen_;
    OpenGL::Camera &camera_;
};
} // namespace OpenGTA

#endif
