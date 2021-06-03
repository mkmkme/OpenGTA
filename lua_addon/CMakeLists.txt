add_library(ogta-lua STATIC EXCLUDE_FROM_ALL
    lua_camera.cpp
    lua_cityview.cpp
    lua_ini_bridge.cpp
    lua_map.cpp
    lua_screen.cpp
    lua_spritecache.cpp
    lua_stackguard.cpp
    lua_vm.cpp)
target_link_libraries(ogta-lua
    ogta
    ogta-gl
    ogta-util
    ${LUA_LIBRARIES}
    ${GLU_LIBRARIES})
