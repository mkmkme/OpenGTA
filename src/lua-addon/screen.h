#pragma once

#include <lua-addon/lua.h>

namespace OpenGL {
class Screen;
}

namespace OpenGTA::Script {
class LuaScreen {
public:
    explicit LuaScreen(OpenGL::Screen &s);
    int registerFunctions(lua_State *L);

private:
    OpenGL::Screen &screen_;
};

} // namespace OpenGTA::Script
