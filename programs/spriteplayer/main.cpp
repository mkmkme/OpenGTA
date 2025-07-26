/************************************************************************
 * Copyright (c) 2005-2007 tok@openlinux.org.uk                          *
 *               2021-2023 mkmkme                                        *
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
#include <memory>

#include <SDL_timer.h>

#define SDL_MAIN_HANDLED

#include <SDL2/SDL_opengl.h>

#include "graphics/camera.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "util/errors.h"
#include "util/file-manager.h"
#include "util/log.h"

#include "core/active-style.h"
#include "core/game_objects.h"
#include "core/graphics-base.h"
#include "core/main-msg-lookup.h"
#include "core/spritemanager.h"

using namespace std::string_view_literals;

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
    bool c_c_ { true };
    int car_model_ { 0 };
    int car_remap_ { -1 };
    bool play_with_car_ { false };
    uint32_t car_delta_ { 0 };

    std::unique_ptr<Car> car_;
    Pedestrian ped_ { glm::vec3 { 0.5f, 0.5f, 0.5f }, PED_POS, 0xffffffff };

    Sprite::SpriteType sprite_type_ { ped_.getSpriteType() };
};

namespace {
// TODO: enum class
std::string_view vtype2name(int vt)
{
    switch (vt) {
        case 0:
            return "bus";
        case 3:
            return "motorcycle";
        case 4:
            return "car";
        case 8:
            return "train";
        default:
            break;
    }
    return "";
}
} // namespace

SpritePlayer::SpritePlayer(OpenGL::Screen &screen, OpenGL::Camera &camera, OpenGL::DrawableFont &font)
    : screen_(screen)
    , camera_(camera)
    , font_(font)
{
}

void SpritePlayer::run()
{
    glClearColor(1, 1, 1, 1);

    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_CULL_FACE);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0);

    camera_.setVectors({ 4, 5, 4 }, { 4, 0.0f, 4.0f }, { 0, 0, -1 });
    camera_.setFollowMode(ped_.pos);

    SDL_Event event;

    while (!done_) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    handleKeyPress(&event.key.keysym);
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    screen_.resize(event.window.data1, event.window.data2);
                    break;
                case SDL_QUIT:
                    done_ = true;
                    break;
                default:
                    break;
            }
        }
        drawScene(SDL_GetTicks());
    }
}

void SpritePlayer::quit() const noexcept
{
    SDL_Quit();
    fmt::println("Goodbye");
}

void SpritePlayer::drawScene(uint32_t now_ticks)
{
    GL_CHECKERROR;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    screen_.set3DProjection();
    camera_.update(now_ticks, screen_);

    if (play_with_car_) {
        if (car_) {
            car_->update(now_ticks);
            OpenGTA::SpriteManager::Instance().draw(*car_);
        }

        screen_.setFlatProjection();

        glPushMatrix();
        glTranslatef(10, 10, 0);

        std::string sprite_info;
        if (car_) {
            sprite_info = fmt::format(
                "{} model: {} name: {}",
                vtype2name(car_->carInfo.vtype),
                car_model_,
                OpenGTA::MainMsgLookup::Instance().get().getText(fmt::format("car{}", car_model_))
            );
        } else {
            sprite_info = "not a model: " + std::to_string(car_model_);
        }
        font_.drawString(sprite_info);
        glPopMatrix();
    } else {
        if (play_anim_ && now_ticks > play_anim_time_ + 200) {
            ++now_frame_;
            if (now_frame_ > second_offset_)
                now_frame_ = first_offset_;
            ped_.getAnimation().firstFrameOffset = now_frame_;
            play_anim_time_ = now_ticks;
        }
        OpenGTA::SpriteManager::Instance().draw(ped_);

        screen_.setFlatProjection();

        glPushMatrix();
        glTranslatef(10, 10, 0);
        std::string sprite_info = std::string { OpenGTA::GraphicsBase::getSpriteName(sprite_type_) } + " offset " +
            std::to_string(frame_offset_);
        font_.drawString(sprite_info);
        glPopMatrix();
    }

    SDL_GL_SwapWindow(screen_.get());
    GL_CHECKERROR;
}

void SpritePlayer::handleKeyPress(SDL_Keysym *keysym)
{
    const auto &style = OpenGTA::ActiveStyle::Instance().get();
    bool update_anim = false;
    switch (keysym->sym) {
        using SpriteType = OpenGTA::GraphicsBase::SpriteNumbers::SpriteType;

        case SDLK_ESCAPE:
            done_ = true;
            break;
        case SDLK_TAB:
            c_c_ = !c_c_;
            glClearColor(c_c_, c_c_, c_c_, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            break;
        case 'k':
            if (car_delta_ > 0)
                car_delta_ -= 1;
            if (car_)
                car_->setDelta(car_delta_);
            break;
        case 'l':
            if (car_delta_ < 32)
                car_delta_ += 1;
            if (car_) {
                car_->setDelta(car_delta_);
            }
            break;
        case '=':
            camera_.translateBy({ 0, -0.5f, 0 });
            break;
        case '-':
            camera_.translateBy({ 0, 0.5f, 0 });
            break;
        case '1':
            if (play_with_car_) {
                if (car_->getAnimState().get_item(1))
                    car_->closeDoor(0);
                else
                    car_->openDoor(0);
            }
            break;
        case '2':
            if (play_with_car_) {
                if (car_->getAnimState().get_item(2))
                    car_->closeDoor(1);
                else
                    car_->openDoor(1);
            }
            break;
        case '3':
            if (play_with_car_) {
                if (car_->getAnimState().get_item(3))
                    car_->closeDoor(2);
                else
                    car_->openDoor(2);
            }
            break;
        case '4':
            if (play_with_car_) {
                if (car_->getAnimState().get_item(4))
                    car_->closeDoor(3);
                else
                    car_->openDoor(3);
            }
            break;

        case ',':
            if (play_with_car_ && car_model_ > 0) {
                car_model_ -= 1;
            }
            if (frame_offset_ > 0) {
                frame_offset_ -= 1;
            }
            update_anim = true;
            break;
        case '.':
            if (play_with_car_ && car_model_ < 88) {
                car_model_ += 1;
            }
            if (frame_offset_ < style.spriteNumbers.countByType(ped_.getSpriteType()) - 1) {
                frame_offset_ += 1;
            }
            update_anim = true;
            break;
        case 'n':
            if (play_with_car_ && car_remap_ > -1) {
                car_remap_ -= 1;
                INFO("remap: {}", car_remap_);
            }
            do {
                if (std::to_underlying(sprite_type_) > 0) {
                    sprite_type_ = SpriteType(std::to_underlying(sprite_type_) - 1);
                }
            } while (style.spriteNumbers.countByType(sprite_type_) == 0);
            ped_.setSpriteType(sprite_type_);
            frame_offset_ = 0;
            update_anim = true;
            break;
        case 'm':
            if (play_with_car_ && car_remap_ < 11) {
                car_remap_ += 1;
                INFO("remap: {}", car_remap_);
            }
            do {
                int spr_type = std::to_underlying(sprite_type_) + 1;
                sprite_type_ = spr_type > 20 ? ped_.getSpriteType() : SpriteType(spr_type);
            } while (style.spriteNumbers.countByType(sprite_type_) == 0);
            ped_.setSpriteType(sprite_type_);
            frame_offset_ = 0;
            update_anim = true;
            break;
        case 's':
            if (play_with_car_) {
                car_->setSirenAnim(true);
            } else {
                WARN("No car to set siren anim on");
            }
            break;
        case SDLK_F2:
            bbox_toggle_ = !bbox_toggle_;
            OpenGTA::SpriteManager::Instance().setDrawBBox(bbox_toggle_);
            break;
        case SDLK_F3:
            texsprite_toggle_ = !texsprite_toggle_;
            OpenGTA::SpriteManager::Instance().setDrawTexBorder(texsprite_toggle_);
            break;
        case SDLK_F5:
            first_offset_ = frame_offset_;
            INFO("First frame: {}", first_offset_);
            break;
        case SDLK_F6:
            second_offset_ = frame_offset_;
            INFO("Last frame: {}", second_offset_);
            break;
        case SDLK_F7:
            play_anim_ = !play_anim_;
            if (play_anim_)
                INFO("Playing: {} .. {}", first_offset_, second_offset_);
            now_frame_ = first_offset_;
            break;
        case SDLK_F8:
            play_with_car_ = !play_with_car_;
            update_anim = true;
            break;
        default:
            break;
    }
    if (update_anim) {
        ped_.setAnimation(OpenGTA::SpriteObject::Animation(frame_offset_, 0));
        if (play_with_car_)
            safeTryLoadCar();
    }
}

void SpritePlayer::safeTryLoadCar() noexcept
{
    try {
        car_ = std::make_unique<OpenGTA::Car>(PED_POS, 0, 0, car_model_, car_remap_);
    } catch (Util::UnknownKey &uk) {
        car_.reset();
        ERROR("not a model");
    }
}

} // namespace OpenGTA

namespace {
inline void usage(const char *a0)
{
    fmt::print("USAGE: {} [style-filename]", a0);
    fmt::print(
        "\nDefault is: STYLE001.G24\n"
        "Keys:\n"
        " + - : zoom in/out\n"
        " , . : previous/next frame offset\n"
        " n m : previous/next sprite-type\n"
        " tab : black/white background\n"
        " F2  : toggle BBox drawn\n"
        " F3  : toggle tex-border drawn\n"
        " F5  : prepare animation: first-frame = current frame\n"
        " F6  : prepare animation: last-frame  = current frame\n"
        " F7  : toggle: play frames\n"
        " F8  : toggle: special-car-mode\n"
        "\nIn car-mode:\n"
        " , . : choose model\n"
        " n m : choose remap\n"
        " 1, 2, 3, 4 : open car door (if exists)\n"
        " s   : toggle siren anim (if exists)\n"
    );
}
} // namespace

int main(int argc, char *argv[])
{
    std::string style_file = "STYLE001.G24";
    if (argc > 2) {
        fmt::print(stderr, "Usage: {} [STYLE_FILENAME]\n", argv[0]);
        return 1;
    }
    if (argc == 2) {
        if (argv[1] == "-h"sv) {
            usage(argv[0]);
            return 0;
        }
        style_file = argv[1];
    }

    const Util::PhysFSContext pfs("spriteplayer");

    OpenGL::Screen screen;
    OpenGL::Camera camera;

    screen.activate(640, 480);

    OpenGTA::ActiveStyle::Instance().load(style_file);
    OpenGTA::ActiveStyle::Instance().get().setDeltaHandling(true);
    OpenGTA::MainMsgLookup::Instance().load("ENGLISH.FXT");

    OpenGL::DrawableFont font { "F_MTEXT.FON", 1 };
    OpenGTA::SpritePlayer player(screen, camera, font);
    player.run();
    player.quit();

    SDL_Quit();

    return 0;
}
