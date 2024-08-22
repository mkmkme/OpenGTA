add_executable(spriteplayer spriteplayer.cpp)
target_include_directories(spriteplayer PRIVATE ${OpenGTA_SOURCE_DIR}/src)
target_link_libraries(spriteplayer PRIVATE
    ogta::coldet-3d
    ogta::core
    ogta::math
    ogta::graphics
    ogta::util
    SDL2::SDL2
    OpenGL::GLU
    physfs
    fmt::fmt)

# TODO: fix this
add_executable(slopeview  EXCLUDE_FROM_ALL display_slopes.cpp common_sdl_gl.cpp)
target_link_libraries(slopeview PRIVATE SDL2::SDL2 OpenGL::GLU physfs)
target_include_directories(slopeview PRIVATE ${OpenGTA_SOURCE_DIR}/src)

# target_link_libraries(slopeview
#     ogta
#     ogta-util
#     SDL2::SDL2
#     OpenGL::GLU
#     physfs)

add_executable(g24-dumper g24-dumper.cpp)
target_link_libraries(g24-dumper PRIVATE
    ogta::core
    ogta::graphics
    ogta::util
    SDL2::SDL2
    OpenGL::GLU
    physfs
    fmt::fmt)


add_executable(font-dump font-dump.cpp)
target_link_libraries(font-dump PRIVATE
    ogta::core
    ogta::graphics
    ogta::util
    SDL2::SDL2
    OpenGL::GLU
    physfs
    fmt::fmt)

# TODO: broken
add_executable(display-font EXCLUDE_FROM_ALL display-font.cpp common_sdl_gl.cpp)
target_link_libraries(display-font PRIVATE
    ogta::core
    ogta::graphics
    ogta::util
    SDL2::SDL2
    OpenGL::GLU
    physfs
    fmt::fmt)

add_executable(obj-dump obj-dump.cpp)
target_link_libraries(obj-dump PRIVATE
    ogta::core
    ogta::graphics
    ogta::util
    SDL2::SDL2
    OpenGL::GLU
    physfs
    fmt::fmt)


# TODO: broken
add_executable(car_dump EXCLUDE_FROM_ALL car_dump.cpp)
target_link_libraries(car_dump PRIVATE
    ogta::core
    ogta::graphics
    ogta::util
    SDL2::SDL2
    OpenGL::GLU
    physfs
    fmt::fmt)

# broken due to missing slope2_data.h
# add_executable(blockview EXCLUDE_FROM_ALL tools/blockview.cpp)

# target_link_libraries(blockview
#     SDL2::SDL2
#     physfs
#     OpenGL::GLU)

# if(SDL_MIXER_FOUND)
#     add_executable(sound_test EXCLUDE_FROM_ALL
#         util/sound_device.cpp
#         util/sound_fx_cache.cpp
#         util/sound_music_player.cpp
#         util/sound_resample2.cpp
#         util/sound_system.cpp
#         util/physfsrwops.c
#         read_sdt.cpp)

#     target_compile_definitions(sound_test PRIVATE SOUND_TEST WITH_SOUND)

#     target_link_libraries(sound_test
#         ogta-util
#         ${SDL2_LIBRARIES}
#         ${SDL_MIXER_LIBRARIES}
#         # SDL_sound-devel is missing pkg-config file on Fedora 34
#         # I would bother adding this myself, but I'm switching to SDL2 anyways
#         SDL_sound
#         ${PHYSFS_LIBRARIES})
# endif()
