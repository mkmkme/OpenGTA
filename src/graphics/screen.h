/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *           (c) 2021-2023 mkmkme                                        *
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

#include <SDL2/SDL_video.h>
namespace OpenGL {

enum class VSyncMode : uint8_t {
    NoSync,
    SDL,
};

class Screen final {
public:
    Screen() noexcept;
    ~Screen();

    Screen(const Screen &copy) = delete;
    Screen &operator=(const Screen &copy) = delete;
    Screen(Screen &&move) = delete;
    Screen &operator=(Screen &&move) = delete;

    void set3DProjection() const noexcept;
    void setFlatProjection() const noexcept;
    void setFullScreenFlag(bool v) noexcept;
    void toggleFullscreen() noexcept;
    void activate(uint32_t w = 0, uint32_t h = 0);
    void resize(uint32_t w, uint32_t h) noexcept;
    static void setSystemMouseCursor(bool visible) noexcept;
    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] bool fullscreen() const noexcept { return video_flags_ & SDL_WINDOW_FULLSCREEN; }
    [[nodiscard]] float fieldOfView() const noexcept { return field_of_view_; }
    [[nodiscard]] float nearPlane() const noexcept { return near_plane_; }
    [[nodiscard]] float farPlane() const noexcept { return far_plane_; }
    [[nodiscard]] SDL_Window *get() noexcept { return window_; }
    void makeScreenshot(const char *filename) const noexcept;
    void setupGlVars(float fov, float near_p, float far_p) noexcept;
    void setVSyncMode(VSyncMode mode) noexcept { vsync_mode_ = mode; }

private:
    void initGL();
    void initScreen(uint32_t w, uint32_t h);

    SDL_Window *window_;
    SDL_GLContext gl_context_ {};
    uint32_t width_, height_;
    uint32_t video_flags_;
    VSyncMode vsync_mode_;
    float field_of_view_;
    float near_plane_;
    float far_plane_;
};
} // namespace OpenGL
