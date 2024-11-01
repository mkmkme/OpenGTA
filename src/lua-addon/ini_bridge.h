#ifndef LUA_INI_BRIDGE_H
#define LUA_INI_BRIDGE_H

#include <string>

#include <core/read_ini.h>

#include <lua-addon/lua.h>

namespace OpenGTA::Script {
class IniScriptBridge : public ScriptParser {
public:
    explicit IniScriptBridge(const std::string &file);
    ~IniScriptBridge() override;
    lua_State *L;
    void loadLevel(UInt32 level);

protected:
    void reset();
    void acceptDefinition(char *);
    void acceptCommand(char *);
};
} // namespace OpenGTA::Script

#endif
