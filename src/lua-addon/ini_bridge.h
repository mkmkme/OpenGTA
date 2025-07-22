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
    void loadLevel(uint32_t level);

protected:
    void reset() const;
    void acceptDefinition(char *def) override;
    void acceptCommand(char *cmd) override;
};
} // namespace OpenGTA::Script

#endif
