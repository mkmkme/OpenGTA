#include <fmt/base.h>

#define SDL_MAIN_HANDLED

#ifdef OGTA_USE_MODERN_GL
#include <glad/gl.h>
#endif

#include "graphics/camera.h"
#include "graphics/font.h"
#include "graphics/screen.h"
#include "util/file-manager.h"

#include "core/active-style.h"
#include "core/graphics-base.h"
#include "core/main-msg-lookup.h"
#include "spriteplayer-shader.h"

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
    // Initialize SDL early to allow setting GL attributes
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        fmt::print(stderr, "Failed to initialize SDL: {}\n", SDL_GetError());
        return 1;
    }

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

    const Util::PhysFSContext pfs("spriteplayer-shader");

    // Request OpenGL 3.3 Core Profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    OpenGL::Screen screen;
    OpenGL::Camera camera;

    screen.activate(640, 480);

    OpenGTA::ActiveStyle::Instance().load(style_file);
    OpenGTA::ActiveStyle::Instance().get().setDeltaHandling(true);
    OpenGTA::MainMsgLookup::Instance().load("ENGLISH.FXT");

    OpenGL::DrawableFont font { "F_MTEXT.FON", 1 };
    OpenGTA::SpritePlayerShader player(screen, camera, font);
    player.run();
    player.quit();

    SDL_Quit();

    return 0;
}
