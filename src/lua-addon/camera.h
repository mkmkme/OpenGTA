#pragma once

#include <lua-addon/lua.h>

namespace OpenGL {
class Camera;
}

namespace OpenGTA::Script {
class LuaCamera {
public:
    explicit LuaCamera(OpenGL::Camera &c);
    int registerFunctions(lua_State *L);

private:
    OpenGL::Camera &camera_;
};
} // namespace OpenGTA::Script
