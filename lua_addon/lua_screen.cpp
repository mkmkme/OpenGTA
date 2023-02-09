#include "lua_screen.h"

#include "gl_screen.h"

namespace {
int getFullscreen(lua_State *L)
{
    bool b = OpenGL::Screen::Instance().fullscreen();
    lua_pushboolean(L, b);
    return 1;
}

int setFullscreen(lua_State *L)
{
    bool b = OpenGL::Screen::Instance().fullscreen();
    bool v = lua_toboolean(L, 1);
    if (b != v)
        OpenGL::Screen::Instance().toggleFullscreen();
    return 0;
}

int makeScreenShot(lua_State *L)
{
    OpenGL::Screen::Instance().makeScreenshot(luaL_checkstring(L, 1));
    return 0;
}

const luaL_Reg screen_methods[] = {
    { "getFullscreen", getFullscreen },
    { "setFullscreen", setFullscreen },
    { "makeScreenShot", makeScreenShot },
    { NULL, NULL },
};
} // namespace

int OpenGTA::Script::luaopen_screen(lua_State *L)
{
    luaL_newlib(L, screen_methods);
    return 1;
}