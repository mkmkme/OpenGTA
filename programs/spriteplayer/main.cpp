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

#include <SDL_timer.h>

#include <fmt/base.h>

#define SDL_MAIN_HANDLED

#include <SDL2/SDL_opengl.h>

#include "graphics/camera.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "util/file-manager.h"

#include "core/active-style.h"
#include "core/graphics-base.h"
#include "core/main-msg-lookup.h"
#include "spriteplayer.h"

using namespace std::string_view_literals;

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
