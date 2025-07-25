#ifndef LUA_STACK_GUARD_H
#define LUA_STACK_GUARD_H

#include <lua-addon/lua.h>

namespace Util {

class LuaStackguard {
public:
    explicit LuaStackguard(lua_State *L);
    ~LuaStackguard();

private:
    int m_top;
    lua_State *m_state;
};

#define LGUARD(L) LuaStackguard guard(L)

} // namespace Util
#endif
