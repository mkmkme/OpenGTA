#pragma once

#include "lua.hpp"

namespace OpenGL {
class Screen;
}

namespace OpenGTA::Script {
class LuaScreen {
public:
    LuaScreen(OpenGL::Screen &s);
    int registerFunctions(lua_State *L);

private:
    OpenGL::Screen &screen_;
};

} // namespace OpenGTA::Script
