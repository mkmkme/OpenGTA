#include "lua-addon/ini_bridge.h"

#include <cassert>
#include <cstring>

namespace OpenGTA::Script {

#define GLOBAL_TABLE(name) \
    lua_newtable(L);       \
    lua_setglobal(L, name)

IniScriptBridge::IniScriptBridge(const std::string &file)
    : ScriptParser(file)
{
    L = luaL_newstate();
    luaL_openlibs(L);
}

IniScriptBridge::~IniScriptBridge()
{
    lua_close(L);
}

void IniScriptBridge::reset() const
{
    lua_settop(L, 0);
    GLOBAL_TABLE("commands");
    GLOBAL_TABLE("definitions");
    lua_gc(L, LUA_GCCOLLECT, 0);
    lua_settop(L, 0);
}

void IniScriptBridge::loadLevel(uint32_t level)
{
    reset();
    ScriptParser::loadLevel(level);
}

void IniScriptBridge::acceptCommand(char *cmd)
{
    char *skip_idx = strchr(cmd, ' ');
    assert(skip_idx);
    *skip_idx = 0;
    int idx = strtol(cmd, nullptr, 10);
    ++skip_idx;
    // INFO << idx << " " << skip_idx << std::endl;
    lua_settop(L, 0);

    lua_getglobal(L, "commands");
    lua_pushinteger(L, idx);
    lua_pushstring(L, skip_idx);
    lua_settable(L, -3);

    lua_settop(L, 0);
}

void IniScriptBridge::acceptDefinition(char *def)
{
    char *skip_idx = strchr(def, ' ');
    assert(skip_idx);
    *skip_idx = 0;
    int idx = strtol(def, nullptr, 10);
    skip_idx++;
    if (*skip_idx == '1' && *(skip_idx + 1) == ' ')
        skip_idx += 2;
    // INFO << idx << " " << skip_idx << std::endl;

    lua_settop(L, 0);

    lua_getglobal(L, "definitions");
    lua_pushinteger(L, idx);
    lua_pushstring(L, skip_idx);
    lua_settable(L, -3);

    lua_settop(L, 0);
}

} // namespace OpenGTA::Script
