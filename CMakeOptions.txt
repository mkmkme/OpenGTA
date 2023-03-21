option(FORCE_COLORED_OUTPUT
    "Always produce ANSI-colored output (GNU/Clang only)."
    TRUE)

set(OGTA_VERSION "2007-06-14" CACHE STRING "OpenGTA version to use")

set(DEFAULT_SCREEN_WIDTH 640 CACHE STRING "Default screen width")
set(DEFAULT_SCREEN_HEIGHT 480 CACHE STRING "Default screen height")

set(DEFAULT_SCREEN_VSYNC 2 CACHE STRING
    "Default screen vsync mode: 0 - disabled, 1 - SDL_GL_SWAP_CONTROL, 2 - native call")

set(DEFAULT_DATA_PATH "gtadata.zip" CACHE STRING "OpenGTA default data path")
set(DEFAULT_MOD_PATH "" CACHE STRING "OpenGTA default mod path")
set(DEFAULT_HOME_PATH PHYSFS_getBaseDir() CACHE STRING "OpenGTA default home path")

option(DO_SCALE2X "Enable scale 2x" TRUE)
option(WITH_SOUND "Enable sound" ${SDL_MIXER_FOUND})

option(HAVE_SDL_VSYNC "" TRUE)

option(LOG_USE_ANSI_COLORS "Use ANSI colors for log output" TRUE)
