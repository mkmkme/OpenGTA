#include <string>

#ifdef _MSC_VER
#define SDL_MAIN_HANDLED
#endif

#include <core/dataholder.h>
#include <fmt/core.h>

#include <graphics/camera.h>
#include <graphics/screen.h>
#include <lua-addon/vm.h>
#include <util/file-manager.h>
#include <util/file_helper.h>
#include <util/log.h>

std::string map_filename;
const char *script_file;
int global_Done;

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fmt::print("Usage: {} <map_filename> <script_file>\n", argv[0]);
        return 1;
    }
    map_filename = std::string(argv[1]);
    script_file = argv[2];

    Util::PhysFSContext pfs(argv[0]);
    OpenGL::Screen screen;
    OpenGL::Camera camera;

    screen.activate(640, 480);

    OpenGTA::Script::LuaVM vm { screen, camera };
    OpenGTA::MainMsgLookup::Instance().load("ENGLISH.FXT");
    OpenGTA::ActiveMap::Instance().load(map_filename);
    OpenGTA::Map &loadedMap = OpenGTA::ActiveMap::Instance().get();
    vm.setMap(loadedMap);

    vm.runFile(script_file);

    return 0;
}
