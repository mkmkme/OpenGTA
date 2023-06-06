#pragma once

#include "lua.hpp"

namespace OpenGL {
class Camera;
}

namespace OpenGTA::Script {
class LuaCamera {
public:
    LuaCamera(OpenGL::Camera &c);
    int registerFunctions(lua_State *L);

private:
    OpenGL::Camera &camera_;
};
} // namespace OpenGTA::Script
