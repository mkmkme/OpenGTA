#include <string>
#include "lua_vm.h"
#include "lunar.h"
#include "lua_map.h"
#include "lua_cityview.h"
#include "lua_stackguard.h"
#include "lua_camera.h"
#include "lua_screen.h"
#include "lua_spritecache.h"
#include "m_exceptions.h"

using namespace Util;

extern int global_Done;

  namespace OpenGTA::Script {
    LuaVM::LuaVM(OpenGL::Screen &screen, OpenGL::Camera &camera)
    : L(nullptr) , screen_(screen), camera_(camera) {
      L = luaL_newstate();
      if (L == nullptr)
        throw E_SCRIPTERROR("Failed to create Lua state!");

      luaL_requiref(L, "base", luaopen_base, 1);
      luaL_requiref(L, "math", luaopen_math, 1);
      luaL_requiref(L, "table", luaopen_table, 1);
      _registered = false;
      lua_settop(L, 0);
      prepare();
    }

    LuaVM::~LuaVM() {
      if (L != nullptr)
        lua_close(L);
      L = nullptr;
    }

    int vm_quit([[maybe_unused]] lua_State *L) {
      global_Done = true;
      return 0;
    }

    void LuaVM::prepare() {
      LGUARD(L);
      if (!_registered) {
        Lunar<Block>::Register2(L);
        Lunar<LMap>::Register2(L);
        Lunar<CityView>::Register2(L);
        lua_newtable(L);
        camera_.registerFunctions(L);
        screen_.registerFunctions(L);
        luaL_requiref(L, "spritecache", luaopen_spritecache, 1);
        lua_pushcfunction(L, vm_quit);
        lua_setglobal(L, "quit");
      }
      _registered = true;
    }

    lua_State* LuaVM::getInternalState() {
      return(L);
    }

    void LuaVM::setMap(OpenGTA::Map & map) {
      LGUARD(L);
      LMap * mptr = static_cast<LMap*>(&map);
      lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      int scv_ref = Lunar<LMap>::push(L, mptr, false);
      lua_pushvalue(L, scv_ref);
      lua_setglobal(L, "map");
    }

    void LuaVM::setCityView(OpenGTA::CityView & cv) {
#ifndef LUA_MAP_ONLY
      LGUARD(L);
      CityView *scv = static_cast<CityView*>(&cv);
      lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
      int scv_ref = Lunar<CityView>::push(L, scv, false);
      lua_pushvalue(L, scv_ref);
      lua_setglobal(L, "city_view");
#endif
    }

    void LuaVM::runString(const char* _str) {
      LGUARD(L);
      if (!_str)
        return;
      if (luaL_loadbuffer(L, _str, strlen(_str), "cmd") ||
        lua_pcall(L, 0, 0, 0))
        throw E_SCRIPTERROR("Error running string: " + std::string(lua_tostring(L, -1)));
    }

    void LuaVM::runFile(const char* filename) {
      LGUARD(L);
      if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0))
        throw E_SCRIPTERROR("Error running file: " + std::string(lua_tostring(L, -1)));
    }

    void LuaVM::callSimpleFunction(const char* func_name) {
      LGUARD(L);
      lua_getglobal(L, func_name);
      if (lua_type(L, -1) == LUA_TFUNCTION) {
        if (lua_pcall(L, 0, 0, 0) != 0)
          throw E_SCRIPTERROR(("Exception calling function: ") + std::string(lua_tostring(L, -1)));
      }
      else
        throw E_SCRIPTERROR("No such function: " + std::string(func_name));
    }

    int LuaVM::getGlobalInt(const char* key) {
      LGUARD(L);
      lua_getglobal(L, key);
      if (!lua_isnumber(L, -1))
        throw E_SCRIPTERROR("Expected int value for key: " + std::string(key));
      int v = int(lua_tointeger(L, -1));
      return v;
    }

    float LuaVM::getGlobalFloat(const char* key) {
      LGUARD(L);
      lua_getglobal(L, key);
      if (!lua_isnumber(L, -1))
        throw E_SCRIPTERROR("Expected float value for key: " + std::string(key));
      float v = float(lua_tonumber(L, -1));
      return v;
    }

    const char* LuaVM::getGlobalString(const char* key) {
      LGUARD(L);
      lua_getglobal(L, key);
      if (!lua_isstring(L, -1))
        throw E_SCRIPTERROR("Expected string value for key: " + std::string(key));
      const char* v = lua_tostring(L, -1);
      return v;
    }

    bool LuaVM::getGlobalBool(const char* key) {
      LGUARD(L);
      lua_getglobal(L, key);
      if (!lua_isboolean(L, -1))
        throw E_SCRIPTERROR("Expected boolean value for key: " + std::string(key));
      return lua_toboolean(L, -1);
    }

    void LuaVM::setInt(const char* key, int v) {
      lua_pushinteger(L, v);
      lua_setfield(L, -2, key);
    }

    int LuaVM::getInt(const char* key) {
      int ret;
      if (tryGetInt(key, ret))
        return ret;
      throw E_SCRIPTERROR("Expected int value for key: " + std::string(key));
    }

    bool LuaVM::tryGetInt(const char *key, int &buf) noexcept
    {
      LGUARD(L);
      lua_getfield(L, -1, key);
      if (!lua_isnumber(L, -1))
        return false;
      buf = luaL_checkinteger(L, -1);
      return true;
    }

    void LuaVM::setFloat(const char* key, float v) {
      lua_pushnumber(L, v);
      lua_setfield(L, -2, key);
    }

    float LuaVM::getFloat(const char* key) {
      float ret;
      if (tryGetFloat(key, ret))
        return ret;
      throw E_SCRIPTERROR("Expected float value for key: " + std::string(key));
    }

    bool LuaVM::tryGetFloat(const char *key, float &buf) noexcept
    {
      LGUARD(L);
      lua_getfield(L, -1, key);
      if (!lua_isnumber(L, -1))
        return false;
      buf = luaL_checknumber(L, -1);
      return true;
    }

    void LuaVM::setString(const char* key, const char* v) {
      lua_pushstring(L, v);
      lua_setfield(L, -2, key);
    }

    const char* LuaVM::getString(const char* key) {
      LGUARD(L);
      lua_getfield(L, -1, key);
      if (!lua_isstring(L, -1))
        throw E_SCRIPTERROR("Expected string value for key: " + std::string(key));
      return luaL_checkstring(L, -1);
    }

    void LuaVM::setBool(const char* key, bool v) {
      lua_pushboolean(L, v);
      lua_setfield(L, -2, key);
    }

    bool LuaVM::getBool(const char* key) {
      bool ret;
      if (tryGetBool(key, ret))
        return ret;
      throw E_SCRIPTERROR("Expected boolean value for key: " + std::string(key));
    }

    bool LuaVM::tryGetBool(const char *key, bool &buf) noexcept
    {
      LGUARD(L);
      lua_getfield(L, -1, key);
      if (!lua_isboolean(L, -1))
        return false;
      buf = lua_toboolean(L, -1);
      return true;
    }

    void LuaVM::setGlobalInt(const char* key, int v) {
      LGUARD(L);
      lua_pushinteger(L, v);
      lua_setglobal(L, key);
    }

    void LuaVM::setGlobalFloat(const char* key, float v) {
      LGUARD(L);
      lua_pushnumber(L, v);
      lua_setglobal(L, key);
    }

    void LuaVM::setGlobalString(const char* key, const char* v) {
      LGUARD(L);
      lua_pushstring(L, v);
      lua_setglobal(L, key);
    }

    void LuaVM::setGlobalBool(const char* key, bool v) {
      LGUARD(L);
      lua_pushboolean(L, v);
      lua_setglobal(L, key);
    }

  }

