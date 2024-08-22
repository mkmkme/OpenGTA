add_library(ogta_lua_addon_ OBJECT
    camera.cpp
    cityview.cpp
    ini_bridge.cpp
    map.cpp
    screen.cpp
    spritecache.cpp
    stackguard.cpp
    vm.cpp)
target_include_directories(ogta_lua_addon_ PUBLIC
    ${OpenGTA_SOURCE_DIR}/src)
target_link_libraries(ogta_lua_addon_ PUBLIC ogta::math)

add_library(ogta::lua-addon ALIAS ogta_lua_addon_)
