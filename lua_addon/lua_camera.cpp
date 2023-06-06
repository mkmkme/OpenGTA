#include "lua_camera.h"

#include "gl_camera.h"

namespace {
int setSpeed(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    float tmp = float(luaL_checknumber(L, 1));
    c->setSpeed(tmp);
    return 0;
}

int setRotating(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    bool b = lua_toboolean(L, 1);
    c->setRotating(b);
    return 0;
}

int getEye(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    Vector3D &e = c->getEye();
    lua_pushnumber(L, e.x);
    lua_pushnumber(L, e.y);
    lua_pushnumber(L, e.z);
    return 3;
}

int setEye(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    Vector3D &e = c->getEye();
    e.x = luaL_checknumber(L, 1);
    e.y = luaL_checknumber(L, 2);
    e.z = luaL_checknumber(L, 3);
    return 0;
}

int getCenter(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    Vector3D &e = c->getCenter();
    lua_pushnumber(L, e.x);
    lua_pushnumber(L, e.y);
    lua_pushnumber(L, e.z);
    return 3;
}

int setCenter(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    Vector3D &e = c->getCenter();
    e.x = luaL_checknumber(L, 1);
    e.y = luaL_checknumber(L, 2);
    e.z = luaL_checknumber(L, 3);
    return 0;
}

int getUp(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    Vector3D &e = c->getUp();
    lua_pushnumber(L, e.x);
    lua_pushnumber(L, e.y);
    lua_pushnumber(L, e.z);
    return 3;
}

int setUp(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    Vector3D &e = c->getUp();
    e.x = luaL_checknumber(L, 1);
    e.y = luaL_checknumber(L, 2);
    e.z = luaL_checknumber(L, 3);
    return 0;
}

int setGravityOn(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    bool v = lua_toboolean(L, 1);
    c->setCamGravity(v);
    return 0;
}

int interpolateToPosition(lua_State *L)
{
    auto *c = static_cast<OpenGL::Camera *>(lua_touserdata(L, lua_upvalueindex(1)));
    auto x = float(luaL_checknumber(L, 1));
    auto y = float(luaL_checknumber(L, 2));
    auto z = float(luaL_checknumber(L, 3));
    Uint32 msecInterval = Uint32(luaL_checkinteger(L, 4));
    c->interpolate(Vector3D(x, y, z), 1, msecInterval);
    return 0;
}

} // namespace

OpenGTA::Script::LuaCamera::LuaCamera(OpenGL::Camera &c)
    : camera_(c)
{
}

int OpenGTA::Script::LuaCamera::registerFunctions(lua_State *L)
{
    // Put all functions into a table
    lua_createtable(L, 0, 10);

    // Add the functions to the table
    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, setSpeed, 1);
    lua_setfield(L, -2, "setSpeed");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, setRotating, 1);
    lua_setfield(L, -2, "setRotating");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, getEye, 1);
    lua_setfield(L, -2, "getEye");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, setEye, 1);
    lua_setfield(L, -2, "setEye");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, getCenter, 1);
    lua_setfield(L, -2, "getCenter");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, setCenter, 1);
    lua_setfield(L, -2, "setCenter");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, getUp, 1);
    lua_setfield(L, -2, "getUp");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, setUp, 1);
    lua_setfield(L, -2, "setUp");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, setGravityOn, 1);
    lua_setfield(L, -2, "setGravityOn");

    lua_pushlightuserdata(L, &camera_);
    lua_pushcclosure(L, interpolateToPosition, 1);
    lua_setfield(L, -2, "interpolateToPosition");

    // Set the table as the global "Camera" object
    lua_setglobal(L, "Camera");

    return 0;
}
