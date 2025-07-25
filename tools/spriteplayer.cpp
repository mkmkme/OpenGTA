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
#include <array>
#include <memory>

#ifdef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include <SDL2/SDL_opengl.h>
#include <core/active-style.h>
#include <core/game_objects.h>
#include <core/graphics-base.h>
#include <core/main-msg-lookup.h>
#include <core/spritemanager.h>

#include <graphics/camera.h>
#include <graphics/font.h>
#include <graphics/screen.h>
#include <util/errors.h>
#include <util/file-manager.h>
#include <util/log.h>

using namespace std::string_view_literals;

bool done = false;

std::unique_ptr<OpenGTA::Car> car;
const glm::vec3 _p(4, 0.01f, 4);
OpenGTA::Pedestrian ped(glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(4, 0.01f, 4), 0xffffffff);

int frame_offset = 0;
int first_offset = 0;
int second_offset = 0;
int now_frame = 0;
bool play_anim = false;
unsigned int play_anim_time = 0;
bool bbox_toggle = false;
bool texsprite_toggle = false;
bool c_c = true;
int car_model = 0;
int car_remap = -1;
bool playWithCar = false;
uint32_t car_delta = 0;

int spr_type = (int) ped.getSpriteType();

void safe_try_model(uint8_t model_id)
{
    try {
        car = std::make_unique<OpenGTA::Car>(_p, 0, 0, model_id, car_remap);
    } catch (Util::UnknownKey &uk) {
        car.reset();
        ERROR("not a model");
    }
}

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

void drawScene(uint32_t ticks, OpenGL::Screen &screen, OpenGL::Camera &camera, OpenGL::DrawableFont &font)
{
    GL_CHECKERROR;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    screen.set3DProjection();
    camera.update(ticks, screen);

    if (playWithCar) {
        if (car) {
            car->update(ticks);
            OpenGTA::SpriteManager::Instance().draw(*car);
        }

        screen.setFlatProjection();

        glPushMatrix();
        glTranslatef(10, 10, 0);

        std::string sprite_info;
        if (car != nullptr) {
            sprite_info = fmt::format(
                "{} model: {} name: {}",
                vtype2name(car->carInfo.vtype),
                car_model,
                OpenGTA::MainMsgLookup::Instance().get().getText(fmt::format("car{}", car_model))
            );
        } else {
            sprite_info = "not a model: " + std::to_string(car_model);
        }
        font.drawString(sprite_info);
        glPopMatrix();
    } else {
        if (play_anim && ticks > play_anim_time + 200) {
            now_frame++;
            if (now_frame > second_offset)
                now_frame = first_offset;
            ped.getAnimation().firstFrameOffset = now_frame;
            play_anim_time = ticks;
        }
        OpenGTA::SpriteManager::Instance().draw(ped);

        screen.setFlatProjection();

        glPushMatrix();
        glTranslatef(10, 10, 0);
        std::string sprite_info =
            std::string { OpenGTA::GraphicsBase::getSpriteName(spr_type) } + " offset " + std::to_string(frame_offset);
        font.drawString(sprite_info);
        glPopMatrix();
    }

    SDL_GL_SwapWindow(screen.get());
    GL_CHECKERROR;
}

void handleKeyPress(SDL_Keysym *keysym, OpenGL::Camera &camera)
{
    const auto &style = OpenGTA::ActiveStyle::Instance().get();
    bool update_anim = false;
    switch (keysym->sym) {
        using SpriteType = OpenGTA::GraphicsBase::SpriteNumbers::SpriteType;

        case SDLK_ESCAPE:
            done = true;
            break;
        case SDLK_TAB:
            c_c = !c_c;
            glClearColor(c_c, c_c, c_c, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            break;
        case 'k':
            if (car_delta > 0)
                car_delta -= 1;
            if (car)
                car->setDelta(car_delta);
            break;
        case 'l':
            if (car_delta < 32)
                car_delta += 1;
            if (car) {
                car->setDelta(car_delta);
            }
            break;
        case '=':
            camera.translateBy({ 0, -0.5f, 0 });
            break;
        case '-':
            camera.translateBy({ 0, 0.5f, 0 });
            break;
        case '1':
            if (playWithCar) {
                if (car->getAnimState().get_item(1))
                    car->closeDoor(0);
                else
                    car->openDoor(0);
            }
            break;
        case '2':
            if (playWithCar) {
                if (car->getAnimState().get_item(2))
                    car->closeDoor(1);
                else
                    car->openDoor(1);
            }
            break;
        case '3':
            if (playWithCar) {
                if (car->getAnimState().get_item(3))
                    car->closeDoor(2);
                else
                    car->openDoor(2);
            }
            break;
        case '4':
            if (playWithCar) {
                if (car->getAnimState().get_item(4))
                    car->closeDoor(3);
                else
                    car->openDoor(3);
            }
            break;

        case ',':
            if (playWithCar && car_model > 0) {
                car_model -= 1;
            }
            if (frame_offset > 0) {
                frame_offset -= 1;
            }
            update_anim = true;
            break;
        case '.':
            if (playWithCar && car_model < 88) {
                car_model += 1;
            }
            if (frame_offset < style.spriteNumbers.countByType(ped.getSpriteType()) - 1) {
                frame_offset += 1;
            }
            update_anim = true;
            break;
        case 'n':
            if (playWithCar && car_remap > -1) {
                car_remap -= 1;
                INFO("remap: {}", car_remap);
            }
            do {
                if (spr_type > 0) {
                    spr_type -= 1;
                }
            } while (style.spriteNumbers.countByType((SpriteType) spr_type) == 0);
            ped.setSpriteType((SpriteType) spr_type);
            frame_offset = 0;
            update_anim = true;
            break;
        case 'm':
            if (playWithCar && car_remap < 11) {
                car_remap += 1;
                INFO("remap: {}", car_remap);
            }
            do {
                spr_type += 1;
                if (spr_type > 20)
                    spr_type = (int) ped.getSpriteType();
            } while (style.spriteNumbers.countByType((SpriteType) spr_type) == 0);
            ped.setSpriteType((SpriteType) spr_type);
            frame_offset = 0;
            update_anim = true;
            break;
        case 's':
            if (playWithCar) {
                car->setSirenAnim(true);
            } else {
                WARN("No car to set siren anim on");
            }
            break;
        case SDLK_F2:
            bbox_toggle = !bbox_toggle;
            OpenGTA::SpriteManager::Instance().setDrawBBox(bbox_toggle);
            break;
        case SDLK_F3:
            texsprite_toggle = !texsprite_toggle;
            OpenGTA::SpriteManager::Instance().setDrawTexBorder(texsprite_toggle);
            break;
        case SDLK_F5:
            first_offset = frame_offset;
            INFO("First frame: {}", first_offset);
            break;
        case SDLK_F6:
            second_offset = frame_offset;
            INFO("Last frame: {}", second_offset);
            break;
        case SDLK_F7:
            play_anim = !play_anim;
            if (play_anim)
                INFO("Playing: {} .. {}", first_offset, second_offset);
            now_frame = first_offset;
            break;
        case SDLK_F8:
            playWithCar = !playWithCar;
            update_anim = true;
            break;
        default:
            break;
    }
    if (update_anim) {
        ped.setAnimation(OpenGTA::SpriteObject::Animation(frame_offset, 0));
        if (playWithCar)
            safe_try_model(car_model);
    }
}

void usage(const char *a0)
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

void main_loop(OpenGL::Screen &screen, OpenGL::Camera &camera, OpenGL::DrawableFont &font)
{
    SDL_Event event;

    while (!done) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_KEYDOWN:
                    handleKeyPress(&event.key.keysym, camera);
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    screen.resize(event.window.data1, event.window.data2);
                    break;
                case SDL_QUIT:
                    done = true;
                    break;
                default:
                    break;
            }
        }
        const auto now_ticks = SDL_GetTicks();
        drawScene(now_ticks, screen, camera, font);
    }
}

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
    const Util::PhysFSContext pfs("mapview");

    OpenGL::Screen screen;
    OpenGL::Camera camera;

    screen.activate(640, 480);

    OpenGTA::ActiveStyle::Instance().load(style_file);
    OpenGTA::ActiveStyle::Instance().get().setDeltaHandling(true);
    OpenGTA::MainMsgLookup::Instance().load("ENGLISH.FXT");

    OpenGL::DrawableFont font { "F_MTEXT.FON", 1 };
    glClearColor(1, 1, 1, 1);
    if (playWithCar) {
        car = std::make_unique<OpenGTA::Car>(_p, 0, 0, car_model);
    }

    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT, GL_FILL);
    glEnable(GL_CULL_FACE);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0);

    camera.setVectors({ 4, 5, 4 }, { 4, 0.0f, 4.0f }, { 0, 0, -1 });
    camera.setFollowMode(ped.pos);

    main_loop(screen, camera, font);

    SDL_Quit();

    return 0;
}
