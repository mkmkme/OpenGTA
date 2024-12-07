enable_testing()
include(GoogleTest)
add_executable(tests bitwise-test.cpp)
target_include_directories(tests PRIVATE ${OpenGTA_SOURCE_DIR}/src)
target_link_libraries(tests gtest_main gtest)
gtest_discover_tests(tests)

if(SDL2_image_FOUND)
    add_executable(menudemo menudemo.cpp)
    target_include_directories(menudemo PRIVATE ${PHYSFS_INCLUDE_DIR})
    target_link_libraries(menudemo PRIVATE
        ogta::core
        ogta::graphics
        ogta::lua-addon
        ogta::util
        fmt::fmt
        OpenGL::GLU
        SDL2::SDL2
        SDL2_image::SDL2_image
        ${PHYSFS_LIBRARY}
        ${LUA_LIBRARIES})
endif()

add_executable(lua_map_test lua_map_test.cpp)
target_include_directories(lua_map_test PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(lua_map_test PRIVATE
    ogta::core
    ogta::graphics
    ogta::lua-addon
    ogta::util
    fmt::fmt
    OpenGL::GLU
    SDL2::SDL2
    ${PHYSFS_LIBRARY}
    ${LUA_LIBRARIES})

# broken due to missing tests/test_plane.cpp
# add_executable(plane_test
#     tests/test_plane.cpp)
