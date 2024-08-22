enable_testing()
include(GoogleTest)
add_executable(tests bitwise-test.cpp)
target_include_directories(tests PRIVATE ${OpenGTA_SOURCE_DIR}/src)
target_link_libraries(tests gtest_main gtest)
gtest_discover_tests(tests)

# TODO: broken
# add_executable(lua_map_test lua_map_test.cpp)
# target_link_libraries(lua_map_test PRIVATE ogta::core ogta::lua-addon)

# broken due to missing tests/test_plane.cpp
# add_executable(plane_test
#     tests/test_plane.cpp)
