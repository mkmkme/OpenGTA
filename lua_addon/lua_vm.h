#ifndef OPENGTA_SCRIPT_VM_H
#define OPENGTA_SCRIPT_VM_H

#include "lua.hpp"
#include "lua_camera.h"
#include "lua_screen.h"
#include "gl_cityview.h"

namespace OpenGL {
class Screen;
class Camera;
}

namespace OpenGTA {
  namespace Script {
    class LuaVM {
      public:
        LuaVM(OpenGL::Screen &, OpenGL::Camera &);
        ~LuaVM();

        LuaVM(const LuaVM& copy) = delete;
        LuaVM& operator=(const LuaVM& copy) = delete;
        LuaVM(const LuaVM&& move) = delete;
        LuaVM& operator=(const LuaVM&& move) = delete;

        void  runString(const char*);
        void  runFile(const char*);
        void  callSimpleFunction(const char*);
        void  setCityView(OpenGTA::CityView &);
        void  setMap(OpenGTA::Map &);
        int   getGlobalInt(const char*);
        float getGlobalFloat(const char*);
        const char* getGlobalString(const char*);
        bool  getGlobalBool(const char*);
        void  setGlobalInt(const char*, int);
        void  setGlobalFloat(const char*, float);
        void  setGlobalString(const char*, const char*);
        void  setGlobalBool(const char*, bool);

        void  setInt(const char*, int);
        int   getInt(const char*);
        bool  tryGetInt(const char*, int&) noexcept;
        void  setFloat(const char*, float);
        float getFloat(const char*);
        bool  tryGetFloat(const char*, float&) noexcept;
        void  setString(const char*, const char*);
        const char*  getString(const char*);
        void  setBool(const char*, bool);
        bool  getBool(const char*);
        bool  tryGetBool(const char*, bool&) noexcept;

        lua_State *getInternalState();
      private:
        lua_State *L;
        LuaScreen screen_;
        LuaCamera camera_;
        bool _registered;
        void prepare();
    };
  }
}
#endif
