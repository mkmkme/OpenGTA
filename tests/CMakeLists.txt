enable_testing()
include(GoogleTest)
add_executable(tests bitwise-test.cpp)
target_include_directories(tests PRIVATE ${OpenGTA_SOURCE_DIR}/src)
target_link_libraries(tests gtest_main gtest)
gtest_discover_tests(tests)

if(SDL2_image_FOUND)
    add_executable(menudemo menudemo.cpp)
    target_link_libraries(menudemo PRIVATE
        ogta::core
        ogta::graphics
        ogta::lua-addon
        ogta::util
        fmt::fmt
        OpenGL::GLU
        physfs
        SDL2::SDL2
        SDL2_image::SDL2_image
        ${LUA_LIBRARIES})
endif()

# TODO: broken
# add_executable(lua_map_test lua_map_test.cpp)
# target_link_libraries(lua_map_test PRIVATE ogta::core ogta::lua-addon)

# broken due to missing tests/test_plane.cpp
# add_executable(plane_test
#     tests/test_plane.cpp)
