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
#include "base/config.h"

#ifdef OGTA_USE_MODERN_GL
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>
#endif

#include "graphics/screen.h"
#include "util/errors.h"
#include "util/image_loader.h"
#include "util/log.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <cassert>

#include <SDL2/SDL.h>

namespace {

inline GLboolean queryExtension(const char *extName) noexcept
{
#ifdef OGTA_USE_MODERN_GL
    // Try the legacy way first (returns NULL in Core Profile)
    const char *p = (const char *) glGetString(GL_EXTENSIONS);
    if (p) {
        const char *end = p + strlen(p);
        while (p < end) {
            size_t n = strcspn(p, " ");
            if ((strlen(extName) == n) && (strncmp(extName, p, n) == 0)) {
                return GL_TRUE;
            }
            p += (n + 1);
        }
    } else {
        // Fallback to modern way (OpenGL 3.0+)
        GLint numExtensions = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
        for (int i = 0; i < numExtensions; ++i) {
            const char *extension = (const char *) glGetStringi(GL_EXTENSIONS, i);
            if (extension && strcmp(extension, extName) == 0) {
                return GL_TRUE;
            }
        }
    }
    return GL_FALSE;
#else
    // from the 'Red Book'
    char *p = (char *) glGetString(GL_EXTENSIONS);
    char *end = p + strlen(p);
    while (p < end) {
        size_t n = strcspn(p, " ");
        if ((strlen(extName) == n) && (strncmp(extName, p, n) == 0)) {
            return GL_TRUE;
        }
        p += (n + 1);
    }
    return GL_FALSE;
#endif
}

inline void checkAndClearSDLError(const char *context) noexcept
{
    std::string_view sdl_err = SDL_GetError();
    if (!sdl_err.empty()) {
        ERROR("SDL complained in context '{}': {}", context, sdl_err);
        SDL_ClearError();
    }
}

} // namespace

namespace OpenGL {

Screen::Screen() noexcept
    : window_ { nullptr }
    , video_flags_ { SDL_WINDOW_OPENGL }

    // FIXME: the below need to be args
    , vsync_mode_ { OGTA_DEFAULT_SCREEN_VSYNC ? VSyncMode::SDL : VSyncMode::NoSync }
    , width_ { OGTA_DEFAULT_SCREEN_WIDTH }
    , height_ { OGTA_DEFAULT_SCREEN_HEIGHT }
    , field_of_view_ { 60.0f }
    , near_plane_ { 0.1f }
    , far_plane_ { 250.0f }
{
}

void Screen::activate(uint32_t w, uint32_t h)
{
    if (w)
        width_ = w;
    if (h)
        height_ = h;
    initScreen(w, h);
    INFO("activating screen: {}x{}", width_, height_);
    initGL();
    setSystemMouseCursor(false);
}

void Screen::setupGlVars(float fov, float near_p, float far_p) noexcept
{
    field_of_view_ = fov;
    near_plane_ = near_p;
    far_plane_ = far_p;
}

void Screen::setSystemMouseCursor(bool visible) noexcept
{
    SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

void Screen::setFullScreenFlag(bool v) noexcept
{
    if ((v && fullscreen()) || (!v && !fullscreen()))
        return;
    if (v)
        video_flags_ |= SDL_WINDOW_FULLSCREEN;
    else
        video_flags_ ^= SDL_WINDOW_FULLSCREEN;
}

Screen::~Screen()
{
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
    }
    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        SDL_Quit();
    }
}

void Screen::toggleFullscreen() noexcept
{
    if (video_flags_ & SDL_WINDOW_FULLSCREEN)
        video_flags_ ^= SDL_WINDOW_FULLSCREEN;
    else
        video_flags_ |= SDL_WINDOW_FULLSCREEN;
    resize(width_, height_);
}

void Screen::initScreen(uint32_t w, uint32_t h)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw Util::InvalidFormat("SDL_Init failed: {}", SDL_GetError());
    }
    checkAndClearSDLError("SDL_Init");

    // Get desktop display mode to determine BPP
    SDL_DisplayMode mode;
    if (SDL_GetDesktopDisplayMode(0, &mode) != 0) {
        ERROR("SDL_GetDesktopDisplayMode failed: {}", SDL_GetError());
        // Fallback to a safe default if query fails
        mode.format = SDL_PIXELFORMAT_RGB888;
    }
    const auto bpp = SDL_BITSPERPIXEL(mode.format);

    INFO("video-probe (Desktop):");
    INFO(" bpp: {}", bpp);

    auto [r, g, b] = [bpp]() -> std::tuple<int, int, int> {
        switch (bpp) {
            case 32:
            case 24:
                return { 8, 8, 8 };
            case 16:
                return { 5, 6, 5 };
            case 15:
                return { 5, 5, 5 };
            case 8:
                return { 2, 3, 3 };
            default:
                // Fallback: assume 24/32-bit color if unknown
                return { 8, 8, 8 };
        }
    }();

    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, r);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, g);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, b);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    checkAndClearSDLError("SDL_GL_SetAttribute");

    INFO("Creating window {}x{}", w, h);
    window_ = SDL_CreateWindow("OpenGTA", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL);
    checkAndClearSDLError("SDL_CreateWindow");
    assert(window_ != nullptr);

    gl_context_ = SDL_GL_CreateContext(window_);
    assert(gl_context_ != nullptr);
    checkAndClearSDLError("SDL_GL_CreateContext");

#ifdef OGTA_USE_MODERN_GL
    if (!gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress)) {
        throw Util::InvalidFormat("Failed to initialize GLAD");
    }
#endif
}

void Screen::initGL()
{
    GL_CHECKERROR;
    if (vsync_mode_ != VSyncMode::NoSync) {
        INFO("Enabling VSync (via SDL_GL_SetSwapInterval)");
        SDL_GL_SetSwapInterval(1);
    }
    /*
    GLfloat LightAmbient[]  = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat LightPosition[] = { 1.0f, 1.0f, 0.0f, 0.0f };
    */
    // glShadeModel( GL_SMOOTH );
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    /*
    glEnable( GL_LIGHTING );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
    glLightfv( GL_LIGHT0, GL_AMBIENT, LightAmbient );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, LightDiffuse );
    glLightfv( GL_LIGHT0, GL_POSITION, LightPosition );
    glEnable( GL_LIGHT0 );
    */
#ifndef OGTA_USE_MODERN_GL
    glEnable(GL_COLOR_MATERIAL);
#endif
    glCullFace(GL_BACK);
    // glPolygonMode(GL_FRONT, GL_FILL);
    // glPolygonMode(GL_BACK, GL_LINE);
#ifndef OGTA_USE_MODERN_GL
    glEnable(GL_TEXTURE_2D);
#endif

    if (queryExtension("GL_EXT_texture_filter_anisotropic")) {
        GLfloat maxAniso = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        // Consume error if any (e.g. GL_INVALID_ENUM on some drivers)
        if (glGetError() != GL_NO_ERROR) {
            WARN("GL_EXT_texture_filter_anisotropic not supported");
        }
        // if (maxAniso >= 2.0f)
        ImageUtil::supportedMaxAnisoDegree = maxAniso;
        INFO("GL supports anisotropic filtering with degree: {}", maxAniso);
    }

    GL_CHECKERROR;
}

void Screen::resize(uint32_t w, uint32_t h) noexcept
{
    if (h == 0)
        h = 1;
    SDL_SetWindowSize(window_, w, h);

    glViewport(0, 0, w, h);
    width_ = w;
    height_ = h;
    GL_CHECKERROR;
}

void Screen::set3DProjection() const noexcept
{
#ifndef OGTA_USE_MODERN_GL
    float ratio = float(width_) / float(height_);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(field_of_view_, ratio, near_plane_, far_plane_);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

void Screen::setFlatProjection() const noexcept
{
#ifndef OGTA_USE_MODERN_GL
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width_, 0, height_, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

void Screen::makeScreenshot(const char *filename) const noexcept
{
    INFO("saving screen as: {}", filename);
    std::vector<uint8_t> pixels(width_ * height_ * 3);

    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, width_, height_, GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid *>(pixels.data()));

    SDL_Surface *image = SDL_CreateRGBSurface(SDL_SWSURFACE, width_, height_, 24, 255U << 0, 255U << 8, 255U << 16, 0);
    SDL_LockSurface(image);

    auto *imagepixels = reinterpret_cast<uint8_t *>(image->pixels);
    for (int y = (height_ - 1); y >= 0; --y) {
        const auto row_begin = pixels.begin() + y * width_ * 3;
        const auto row_end = row_begin + width_ * 3;

        std::copy(row_begin, row_end, imagepixels);
        imagepixels += image->pitch;
    }
    SDL_UnlockSurface(image);
    SDL_SaveBMP(image, filename);
    SDL_FreeSurface(image);
}

#ifdef OGTA_USE_MODERN_GL
glm::mat4 Screen::getProjectionMatrix() const
{
    const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
    return glm::perspective(glm::radians(field_of_view_), aspect, near_plane_, far_plane_);
}

glm::mat4 Screen::getOrthoMatrix() const
{
    return glm::ortho(0.0f, static_cast<float>(width_), static_cast<float>(height_), 0.0f, -1.0f, 1.0f);
}
#endif

} // namespace OpenGL
