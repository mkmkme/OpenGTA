#include "spriteplayer-shader.h"

#include <fmt/base.h>

#include "graphics/spritecache.h"
#include "util/errors.h"
#include "util/log.h"

#include "core/active-style.h"
#include "core/main-msg-lookup.h"

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

SpritePlayerShader::SpritePlayerShader(OpenGL::Screen &screen, OpenGL::Camera &camera, OpenGL::DrawableFont &font)
    : screen_(screen)
    , camera_(camera)
    , font_(font)
{
    spriteRenderer_ = std::make_unique<OpenGL::SpriteRendererModern>();
    spriteRenderer_->init();
    fontRenderer_ = std::make_unique<OpenGL::FontRendererModern>("F_MTEXT.FON", 1);
}

void SpritePlayerShader::run()
{
    glClearColor(1, 1, 1, 1);

    // Modern GL doesn't use these fixed-function states
    // glEnable(GL_TEXTURE_2D);  // Not needed - handled by shaders
    // glPolygonMode(GL_FRONT, GL_FILL);  // Default in core profile
    glEnable(GL_CULL_FACE);
    // glEnable(GL_ALPHA_TEST);  // Not in Core Profile - handled by shader discard
    // glAlphaFunc(GL_GREATER, 0);  // Replaced by uniform in shader

    glEnable(GL_DEPTH_TEST); // Good practice for 3D

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

void SpritePlayerShader::quit() const noexcept
{
    SDL_Quit();
    fmt::println("Goodbye");
}

void SpritePlayerShader::drawScene(uint32_t now_ticks)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    screen_.set3DProjection();
    camera_.update(now_ticks, screen_);

    // Get matrices for modern rendering
    glm::mat4 projection = screen_.getProjectionMatrix();
    glm::mat4 view = camera_.getViewMatrix();
    glm::mat4 ui_projection = screen_.getOrthoMatrix();
    const glm::vec3 ui_text_color(1.0f, 1.0f, 1.0f);
    const float ui_text_x = 10.0f;
    const float ui_text_y = static_cast<float>(screen_.height()) - 10.0f - static_cast<float>(font_.getHeight());

    if (play_with_car_) {
        if (car_) {
            car_->update(now_ticks);

            // Get sprite info from style
            GraphicsBase &style = ActiveStyle::Instance().get();
            const auto sprNum = style.spriteNumbers.reIndex(car_->getSpriteNumber(), car_->getSpriteType());
            const SpriteInfo &info = style.getSprite(sprNum);
            const float w = float(info.w) / 64.0f;
            const float h = float(info.h) / 64.0f;

            // Get texture from cache
            OpenGL::SpriteIdentifier si(sprNum, car_->getRemap(), car_->getDelta());
            OpenGL::PagedTexture t;
            if (OpenGL::SpriteCache::Instance().has(si))
                t = OpenGL::SpriteCache::Instance().get(si);
            else
                t = OpenGL::SpriteCache::Instance()
                        .create(car_->getSpriteNumber(), car_->getSpriteType(), car_->getRemap(), car_->getDelta());

            // Draw with modern renderer
            spriteRenderer_->drawSprite(t, car_->pos, car_->rot, w, h, view, projection);

            // Draw debug visualization if enabled
            if (bbox_toggle_) {
                spriteRenderer_->drawBBox(car_->pos, car_->extent(), car_->rot, view, projection);
            }
            if (texsprite_toggle_) {
                spriteRenderer_->drawTextureBorder(car_->pos, car_->rot, w, h, view, projection);
            }
        }

        screen_.setFlatProjection();

        // Modern Font Rendering
        if (fontRenderer_) {
            std::string label;
            if (car_) {
                label = fmt::format(
                    "{} model: {} name: {}",
                    vtype2name(car_->carInfo.vtype),
                    car_model_,
                    OpenGTA::MainMsgLookup::Instance().get().getText(fmt::format("car{}", car_model_))
                );
            } else {
                label = fmt::format("not a model: {}", car_model_);
            }

            fontRenderer_->renderText(label, ui_text_x, ui_text_y, 1.0f, ui_text_color, ui_projection);
        }
    } else {
        if (play_anim_ && now_ticks > play_anim_time_ + 200) {
            ++now_frame_;
            if (now_frame_ > second_offset_)
                now_frame_ = first_offset_;
            ped_.getAnimation().firstFrameOffset = now_frame_;
            play_anim_time_ = now_ticks;
        }

        // Get pedestrian sprite info
        GraphicsBase &style = ActiveStyle::Instance().get();
        uint16_t sprNum = style.spriteNumbers.reIndex(
            ped_.getSpriteNumber() + ped_.getAnimation().firstFrameOffset + ped_.getAnimation().currentFrame,
            ped_.getSpriteType()
        );
        const SpriteInfo &info = style.getSprite(sprNum);
        const float w = float(info.w) / 64.0f;
        const float h = float(info.h) / 64.0f;

        // Get texture from cache
        OpenGL::PagedTexture t;
        if (OpenGL::SpriteCache::Instance().has(sprNum, ped_.getRemap()))
            t = OpenGL::SpriteCache::Instance().get(sprNum, ped_.getRemap());
        else
            t = OpenGL::SpriteCache::Instance().create(
                ped_.getSpriteNumber() + ped_.getAnimation().firstFrameOffset + ped_.getAnimation().currentFrame,
                ped_.getSpriteType(),
                ped_.getRemap()
            );

        // Draw with modern renderer
        spriteRenderer_->drawSprite(t, ped_.pos, ped_.rot, w, h, view, projection);

        // Draw debug visualization if enabled
        if (bbox_toggle_) {
            spriteRenderer_->drawBBox(ped_.pos, ped_.extent(), ped_.rot, view, projection);
        }
        if (texsprite_toggle_) {
            spriteRenderer_->drawTextureBorder(ped_.pos, ped_.rot, w, h, view, projection);
        }

        screen_.setFlatProjection();

        // Modern Font Rendering
        if (fontRenderer_) {
            std::string label =
                fmt::format("{} offset {}", OpenGTA::GraphicsBase::getSpriteName(ped_.getSpriteType()), frame_offset_);
            fontRenderer_->renderText(label, ui_text_x, ui_text_y, 1.0f, ui_text_color, ui_projection);
        }
    }

    SDL_GL_SwapWindow(screen_.get());
}

void SpritePlayerShader::handleKeyPress(SDL_Keysym *keysym)
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
            if (car_) {
                const auto car_delta = car_->getDelta();
                if (car_delta > 0)
                    car_->setDelta(car_delta - 1);
            } else {
                log::error("No car to set delta");
            }
            break;
        case 'l':
            if (car_) {
                const auto car_delta = car_->getDelta();
                if (car_delta < 32) // FIXME: magic number
                    car_->setDelta(car_delta + 1);
            } else {
                log::error("No car to set delta");
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
            if (play_with_car_) {
                log::warn("changing remap for car sprite, but it does not do anything");
                const auto remap = car_->getRemap();
                if (remap > 0) {
                    car_->setRemap(remap - 1);
                    log::info("remap: {}", car_->getRemap());
                }
            } else {
                auto spr_type = ped_.getSpriteType();
                do {
                    if (std::to_underlying(spr_type) > 0) {
                        spr_type = SpriteType(std::to_underlying(spr_type) - 1);
                    }
                } while (style.spriteNumbers.countByType(spr_type) == 0);
                ped_.setSpriteType(spr_type);
            }
            frame_offset_ = 0;
            update_anim = true;
            break;
        case 'm':
            if (play_with_car_) {
                log::warn("changing remap for car sprite, but it does not do anything");
                const auto remap = car_->getRemap();
                if (remap < 11) {
                    car_->setRemap(remap + 1);
                    log::info("remap: {}", car_->getRemap());
                }
            } else {
                auto spr_type = ped_.getSpriteType();
                do {
                    auto spr_type_int = std::to_underlying(spr_type) + 1;
                    spr_type = spr_type_int > 20 ? ped_.getSpriteType() : SpriteType(spr_type_int);
                } while (style.spriteNumbers.countByType(spr_type) == 0);
                ped_.setSpriteType(spr_type);
            }
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
            break;
        case SDLK_F3:
            texsprite_toggle_ = !texsprite_toggle_;
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

void SpritePlayerShader::safeTryLoadCar() noexcept
{
    try {
        car_.emplace(PED_POS, 0.f, 0, car_model_, 0);
    } catch (Util::UnknownKey &) {
        car_.reset();
        ERROR("not a model");
    }
}

} // namespace OpenGTA
