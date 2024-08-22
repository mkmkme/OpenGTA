add_executable(viewer viewer/main.cpp)
target_link_libraries(viewer PRIVATE
    ogta::coldet-3d
    ogta::core
    ogta::graphics
    ogta::lua-addon
    ogta::math
    ogta::opensteer
    ogta::util
    fmt::fmt
    physfs
    SDL2::SDL2
    OpenGL::GLU
    ${LUA_LIBRARIES})


# TODO: fix this
# add_executable(gfxextract EXCLUDE_FROM_ALL
    # gfx_extract.cpp
    # graphics-24bit.cpp
    # graphics-8bit.cpp
    # map.cpp
    # navdata.cpp
    # dataholder.cpp)

# TODO: refactor sources so this is not needed
# target_link_options(gfxextract PRIVATE
#     -Xlinker --unresolved-symbols
#     -Xlinker ignore-all)

# target_link_libraries(gfxextract
#     ogta-util
#     SDL2::SDL2
#     OpenGL::GLU
#     physfs)
add_executable(gfxextract EXCLUDE_FROM_ALL gfx_extract/main.cpp)

target_include_directories(gfxextract PRIVATE
    ${OpenGTA_SOURCE_DIR}/src
    ${OpenGTA_BINARY_DIR}/src)

target_link_libraries(gfxextract PRIVATE
    SDL2::SDL2
    OpenGL::GLU
    physfs)
