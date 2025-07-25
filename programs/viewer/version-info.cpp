#include "version-info.h"

#include <lua.h>

#include <fmt/base.h>

#include "base/config.h"

namespace OpenGTA::Util {
void printVersionInfo() noexcept
{
#define PRINT_FORMATTED(name, value) fmt::print("{:<19}{}\n", name, value)

    PRINT_FORMATTED("OpenGTA version:", OGTA_VERSION_INFO);
    PRINT_FORMATTED("Lua version:", LUA_RELEASE);

#ifdef OGTA_WITH_SOUND
    PRINT_FORMATTED("sound support:", "yes");
#else
    PRINT_FORMATTED("sound support:", "no");
#endif

#ifdef OGTA_WITH_SDL_IMAGE
    PRINT_FORMATTED("SDL_image support:", "yes");
#else
    PRINT_FORMATTED("SDL_image support:", "no");
#endif

#ifdef OGTA_HAVE_SDL_VSYNC
    PRINT_FORMATTED("vsync support:", "yes");
#else
    PRINT_FORMATTED("vsync support:", "no");
#endif

#ifdef OGTA_DO_SCALE2X
    PRINT_FORMATTED("scale2x support:", "yes");
#else
    PRINT_FORMATTED("scale2x support:", "no");
#endif

#ifdef OGTA_DEFAULT_DATA_PATH
    PRINT_FORMATTED("data-path", "[" OGTA_DEFAULT_DATA_PATH "]");
#endif

#ifdef OGTA_DEFAULT_MOD_PATH
    PRINT_FORMATTED("mod-path", "[" OGTA_DEFAULT_MOD_PATH "]");
#endif

#ifdef OGTA_DEFAULT_GRAPHICS_G24
    PRINT_FORMATTED("default graphics:", "G24 - 24 bit");
#else
    PRINT_FORMATTED("default graphics:", "GRY - 8 bit");
#endif
}
} // namespace OpenGTA::Util
