#include "lua_camera.h"

#include "gl_camera.h"

namespace {
int setSpeed(lua_State *L)
{
    float tmp = float(luaL_checknumber(L, 1));
    OpenGL::Camera::Instance().setSpeed(tmp);
    return 0;
}

int setRotating(lua_State *L)
{
    bool b = lua_toboolean(L, 1);
    OpenGL::Camera::Instance().setRotating(b);
    return 0;
}

int getEye(lua_State *L)
{
    Vector3D &e = OpenGL::Camera::Instance().getEye();
    lua_pushnumber(L, e.x);
    lua_pushnumber(L, e.y);
    lua_pushnumber(L, e.z);
    return 3;
}

int setEye(lua_State *L)
{
    Vector3D &e = OpenGL::Camera::Instance().getEye();
    e.x = luaL_checknumber(L, 1);
    e.y = luaL_checknumber(L, 2);
    e.z = luaL_checknumber(L, 3);
    return 0;
}

int getCenter(lua_State *L)
{
    Vector3D &e = OpenGL::Camera::Instance().getCenter();
    lua_pushnumber(L, e.x);
    lua_pushnumber(L, e.y);
    lua_pushnumber(L, e.z);
    return 3;
}

int setCenter(lua_State *L)
{
    Vector3D &e = OpenGL::Camera::Instance().getCenter();
    e.x = luaL_checknumber(L, 1);
    e.y = luaL_checknumber(L, 2);
    e.z = luaL_checknumber(L, 3);
    return 0;
}

int getUp(lua_State *L)
{
    Vector3D &e = OpenGL::Camera::Instance().getUp();
    lua_pushnumber(L, e.x);
    lua_pushnumber(L, e.y);
    lua_pushnumber(L, e.z);
    return 0; // FIXME: 3
}

int setUp(lua_State *L)
{
    Vector3D &e = OpenGL::Camera::Instance().getUp();
    e.x = luaL_checknumber(L, 1);
    e.y = luaL_checknumber(L, 2);
    e.z = luaL_checknumber(L, 3);
    return 0;
}

int setGravityOn(lua_State *L)
{
    bool v = lua_toboolean(L, 1);
    OpenGL::Camera::Instance().setCamGravity(v);
    return 0;
}

int interpolateToPosition(lua_State *L)
{
    float x, y, z;
    x = float(luaL_checknumber(L, 1));
    y = float(luaL_checknumber(L, 2));
    z = float(luaL_checknumber(L, 3));
    Uint32 msecInterval = Uint32(luaL_checkinteger(L, 4));
    OpenGL::Camera::Instance().interpolate(Vector3D(x, y, z), 1, msecInterval);
    return 0;
}

const luaL_Reg camera_functions[] = {
    { "setSpeed", setSpeed },
    { "setRotating", setRotating },
    { "getEye", getEye },
    { "setEye", setEye },
    { "getCenter", getCenter },
    { "setCenter", setCenter },
    { "getUp", getUp },
    { "setUp", setUp },
    { "setGravityOn", setGravityOn },
    { "interpolateToPosition", interpolateToPosition },
    { NULL, NULL },
};
} // namespace

int OpenGTA::Script::luaopen_camera(lua_State *L)
{
    luaL_newlib(L, camera_functions);
    return 1;
}
