#pragma once

#include <string_view>

#include <SDL_keyboard.h>

#include <glm/ext/vector_float3.hpp>

#include "graphics/camera.h"
#include "graphics/cityview.h"
#include "graphics/screen.h"
#include "lua-addon/vm.h"
#include "util/file-manager.h"
#include "util/gui.h"

#include "core/localplayer.h"
#include "viewer-config.h"

namespace OpenGTA {

class Viewer {
public:
    explicit Viewer(ViewerConfig &&config);

    void run();
    void quit();

    void screenGammaCallback(float v);

private:
    void initialize();

    void drawScene(uint32_t ticks);

    void handleKeyPress(SDL_Keysym *keysym);
    void handleKeyUp(SDL_Keysym *keysym);

    void createPedAt(const glm::vec3 &v);
    void showGammaConfig();

    void createIngameUI();
    void updateIngameUI();
    void removeIngameUI();

    void carToggle(); // TODO: rename
    void drawMapMode();

    void printPosition() const noexcept;

    ViewerConfig config_;

    Util::PhysFSContext physfs_context_;
    GUI::Manager guiManager_;
    OpenGL::Screen screen_;
    OpenGL::Camera camera_;
    Script::LuaVM luaVM_;
    LocalPlayer localPlayer_;
    std::unique_ptr<CityView> city_;

    float screen_gamma_ = 1.0f; // TODO: not used
    bool gamma_slide_ = false;

    std::array<float, 3> map_position_ = { 12.0f, 12.0f, 20.0f };
    bool draw_bbox_ = false;
    bool draw_texture_border_ = false;
    bool draw_heading_arrows_ = false;
    bool follow_mode_ = false;
    bool rotating_mode_ = false;
    bool camera_gravity_ = false;

    uint32_t last_tick_ = 0;
    uint32_t fps_last_tick_ = 0;
    uint32_t script_last_tick_ = 0;
    uint32_t num_frames_drawn_ = 0;
    bool paused_ = false;

    GUI::Label *fps_label_;                // Managed by guiManager_
    GUI::Label *cash_label_;               // Managed by guiManager_
    GUI::AnimStatusDisplay *wanted_level_; // Managed by guiManager_

    glm::vec3 test_dot_; // TODO: remove?
};

} // namespace OpenGTA
