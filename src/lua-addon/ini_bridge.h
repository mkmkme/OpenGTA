#ifndef LUA_INI_BRIDGE_H
#define LUA_INI_BRIDGE_H

#include <string>
#include <core/read_ini.h>
#include <lua-addon/lua.h>

namespace OpenGTA {
  namespace Script {
    class IniScriptBridge : public ScriptParser {
      public:
        IniScriptBridge(const std::string &file);
        ~IniScriptBridge();
        lua_State *L;
        void loadLevel(PHYSFS_uint32 level);
      protected:
        void reset();
        void acceptDefinition(char*);
        void acceptCommand(char*);

    };
  }
}

#endif
