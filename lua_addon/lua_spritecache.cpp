#include "lua_spritecache.h"

#include "gl_spritecache.h"

namespace {
int getScale2x(lua_State *L)
{
    bool b = OpenGL::SpriteCache::Instance().getScale2x();
    lua_pushboolean(L, b);
    return 1;
}
int setScale2x(lua_State *L)
{
    bool b = OpenGL::SpriteCache::Instance().getScale2x();
    bool v = lua_toboolean(L, 1);
    if (b != v)
        OpenGL::SpriteCache::Instance().setScale2x(v);
    return 0;
}
const luaL_Reg spritecache_methods[] = {
    { "getScale2x", getScale2x },
    { "setScale2x", setScale2x },
    { NULL, NULL },
};
} // namespace

int OpenGTA::Script::luaopen_spritecache(lua_State *L)
{
    luaL_newlib(L, spritecache_methods);
    return 1;
}