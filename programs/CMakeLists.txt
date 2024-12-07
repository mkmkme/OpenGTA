add_executable(viewer viewer/main.cpp)
target_include_directories(viewer PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(viewer PRIVATE
    ogta::core
    ogta::graphics
    ogta::lua-addon
    ogta::math
    ogta::opensteer
    ogta::util
    fmt::fmt
    OpenGL::GLU
    ${PHYSFS_LIBRARY}
    ${LUA_LIBRARIES})


# TODO: refactor sources so this is not needed
# target_link_options(gfxextract PRIVATE
#     -Xlinker --unresolved-symbols
#     -Xlinker ignore-all)

# target_link_libraries(gfxextract
#     ogta-util
#     SDL2::SDL2
#     OpenGL::GLU
#     physfs)
add_executable(gfxextract gfx_extract/main.cpp)

target_include_directories(gfxextract PRIVATE
    ${PHYSFS_INCLUDE_DIR}
    ${OpenGTA_SOURCE_DIR}/src
    ${OpenGTA_BINARY_DIR}/src)

target_link_libraries(gfxextract PRIVATE
    ogta::core
    ogta::graphics
    ogta::util
    fmt::fmt
    SDL2::SDL2
    OpenGL::GLU
    ${PHYSFS_LIBRARY})
