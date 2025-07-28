#include "spriteplayer.h"

#include <fmt/base.h>

#include "util/errors.h"
#include "util/log.h"

#include "core/active-style.h"
#include "core/main-msg-lookup.h"
#include "core/spritemanager.h"

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

namespace OpenGTA {

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

        if (car_) {
            font_.drawString(
                "{} model: {} name: {}",
                vtype2name(car_->carInfo.vtype),
                car_model_,
                OpenGTA::MainMsgLookup::Instance().get().getText(fmt::format("car{}", car_model_))
            );
        } else {
            font_.drawString("not a model: {}", car_model_);
        }
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
        font_.drawString("{} offset {}", OpenGTA::GraphicsBase::getSpriteName(sprite_type_), frame_offset_);
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
            base_background_ = !base_background_;
            glClearColor(base_background_, base_background_, base_background_, 0);
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
            SpriteManager::Instance().setDrawBBox(bbox_toggle_);
            break;
        case SDLK_F3:
            texsprite_toggle_ = !texsprite_toggle_;
            SpriteManager::Instance().setDrawTexBorder(texsprite_toggle_);
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
        ped_.setAnimation(SpriteObject::Animation(frame_offset_, 0));
        if (play_with_car_)
            safeTryLoadCar();
    }
}

void SpritePlayer::safeTryLoadCar() noexcept
{
    try {
        car_.emplace(PED_POS, 0, 0, car_model_, car_remap_);
    } catch (Util::UnknownKey &uk) {
        car_.reset();
        ERROR("not a model");
    }
}

} // namespace OpenGTA
