#pragma once

#include "graphics/camera.h"
#include "graphics/font.h"
#include "graphics/screen.h"

#include "core/game_objects.h"

namespace OpenGTA {
class SpritePlayer {
public:
    SpritePlayer(OpenGL::Screen &screen, OpenGL::Camera &camera, OpenGL::DrawableFont &font);

    void run();
    void quit() const noexcept;

private:
    void drawScene(uint32_t now_ticks);
    void handleKeyPress(SDL_Keysym *keysym);
    void safeTryLoadCar() noexcept;

    constexpr static glm::vec3 PED_POS { 4, 0.01f, 4 };

    OpenGL::Screen &screen_;
    OpenGL::Camera &camera_;
    OpenGL::DrawableFont &font_;

    bool done_ { false };

    int frame_offset_ { 0 };
    int first_offset_ { 0 };
    int second_offset_ { 0 };
    int now_frame_ { 0 };
    bool play_anim_ { false };
    uint32_t play_anim_time_ { 0 };
    bool bbox_toggle_ { false };
    bool texsprite_toggle_ { false };
    bool base_background_ { true };
    int car_model_ { 0 };
    int car_remap_ { -1 };
    bool play_with_car_ { false };
    uint32_t car_delta_ { 0 };

    std::optional<Car> car_;
    Pedestrian ped_ { glm::vec3 { 0.5f, 0.5f, 0.5f }, PED_POS, 0xffffffff };

    Sprite::SpriteType sprite_type_ { ped_.getSpriteType() };
};

} // namespace OpenGTA
