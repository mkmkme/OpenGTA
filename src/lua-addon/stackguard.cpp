#include <cassert>

#include <lua-addon/stackguard.h>
#include <util/log.h>

namespace Util {
LuaStackguard::LuaStackguard(lua_State *L)
{
    assert(L);
    m_state = L;
    m_top = lua_gettop(m_state);
}

LuaStackguard::~LuaStackguard()
{
    int now_top = lua_gettop(m_state);
    if (now_top > m_top) {
        WARN("Stack-balance: {} > {}", now_top, m_top);
        lua_settop(m_state, m_top);
    }
}

} // namespace Util
