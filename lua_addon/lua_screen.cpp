#include "lua_screen.h"

#include "gl_screen.h"

namespace {
int getFullscreen(lua_State *L)
{
    auto *s = static_cast<OpenGL::Screen *>(lua_touserdata(L, lua_upvalueindex(1)));
    lua_pushboolean(L, s->fullscreen());
    return 1;
}

int toggleFullscreen(lua_State *L)
{
    auto *s = static_cast<OpenGL::Screen *>(lua_touserdata(L, lua_upvalueindex(1)));
    bool v = lua_toboolean(L, 1);
    if (s->fullscreen() != v)
        s->toggleFullscreen();
    return 0;
}

int makeScreenshot(lua_State *L)
{
    auto *s = static_cast<OpenGL::Screen *>(lua_touserdata(L, lua_upvalueindex(1)));
    s->makeScreenshot(luaL_checkstring(L, 1));
    return 0;
}
} // namespace

OpenGTA::Script::LuaScreen::LuaScreen(OpenGL::Screen &s)
    : screen_(s)
{
}

int OpenGTA::Script::LuaScreen::registerFunctions(lua_State *L)
{

    // Put all functions into a table
    lua_createtable(L, 0, 3);

    // Add the functions to the table
    lua_pushlightuserdata(L, &screen_);
    lua_pushcclosure(L, getFullscreen, 1);
    lua_setfield(L, -2, "fullscreen");

    lua_pushlightuserdata(L, &screen_);
    lua_pushcclosure(L, toggleFullscreen, 1);
    lua_setfield(L, -2, "toggleFullscreen");

    lua_pushlightuserdata(L, &screen_);
    lua_pushcclosure(L, makeScreenshot, 1);
    lua_setfield(L, -2, "makeScreenshot");


    // Set the table as the global "Screen" variable
    lua_setglobal(L, "Screen");

    return 0;
}
