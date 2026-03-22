
# TODO: fix this
add_executable(slopeview EXCLUDE_FROM_ALL display_slopes.cpp common_sdl_gl.cpp)
target_include_directories(slopeview PRIVATE ${OpenGTA_SOURCE_DIR}/src)

target_link_libraries(
  slopeview
  ogta::core
  ogta::graphics
  ogta::util
  SDL2::SDL2
  OpenGL::GLU
  fmt::fmt
  physfs)

add_executable(gltexture-segfault gltexture-segfault.cpp)
target_link_libraries(gltexture-segfault PRIVATE OpenGL::GLU fmt::fmt)

add_executable(g24-dumper g24-dumper.cpp)
target_include_directories(g24-dumper PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(g24-dumper PRIVATE ogta::core ogta::graphics ogta::util
                                         OpenGL::GLU fmt::fmt ${PHYSFS_LIBRARY})

add_executable(font-dump font-dump.cpp)
target_include_directories(font-dump PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(font-dump PRIVATE ogta::core ogta::graphics ogta::util
                                        OpenGL::GLU fmt::fmt ${PHYSFS_LIBRARY})

add_executable(display-font display-font.cpp)
target_include_directories(display-font PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(
  display-font PRIVATE ogta::core ogta::graphics ogta::util OpenGL::GLU
                       fmt::fmt ${PHYSFS_LIBRARY})

add_executable(shaders-test shaders-test.cpp)
target_include_directories(shaders-test PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(shaders-test PRIVATE glad::glad OpenGL::GLU fmt::fmt
                                           SDL2::SDL2 ${PHYSFS_LIBRARY})

if(OGTA_USE_MODERN_GL)
  add_executable(display-font-shader display-font-shader.cpp)
  target_include_directories(display-font-shader PRIVATE ${PHYSFS_INCLUDE_DIR})
  target_link_libraries(
    display-font-shader
    PRIVATE ogta::core
            ogta::graphics_modern
            ogta::util
            glad::glad
            OpenGL::GLU
            fmt::fmt
            ${PHYSFS_LIBRARY})
endif()

add_executable(obj-dump obj-dump.cpp)
target_include_directories(obj-dump PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(obj-dump PRIVATE ogta::core ogta::graphics ogta::util
                                       OpenGL::GLU fmt::fmt ${PHYSFS_LIBRARY})

add_executable(mapinfo mapinfo.cpp)
target_include_directories(mapinfo PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(mapinfo PRIVATE ogta::core ogta::graphics ogta::util
                                      OpenGL::GLU fmt::fmt ${PHYSFS_LIBRARY})

add_executable(minimap minimap.cpp)
target_include_directories(minimap PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(minimap PRIVATE ogta::core ogta::graphics ogta::util
                                      OpenGL::GLU fmt::fmt ${PHYSFS_LIBRARY})

add_executable(car_dump car_dump.cpp)
target_include_directories(car_dump PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(car_dump PRIVATE ogta::core ogta::graphics ogta::util
                                       OpenGL::GLU fmt::fmt ${PHYSFS_LIBRARY})

# broken due to missing slope2_data.h add_executable(blockview EXCLUDE_FROM_ALL
# tools/blockview.cpp)

# target_link_libraries(blockview SDL2::SDL2 physfs OpenGL::GLU)

# if(SDL_MIXER_FOUND) add_executable(sound_test EXCLUDE_FROM_ALL
# util/sound_device.cpp util/sound_fx_cache.cpp util/sound_music_player.cpp
# util/sound_resample2.cpp util/sound_system.cpp util/physfsrwops.c
# read_sdt.cpp)

# target_compile_definitions(sound_test PRIVATE SOUND_TEST WITH_SOUND)

# target_link_libraries(sound_test ogta-util ${SDL2_LIBRARIES}
# ${SDL_MIXER_LIBRARIES} # SDL_sound-devel is missing pkg-config file on Fedora
# 34 # I would bother adding this myself, but I'm switching to SDL2 anyways
# SDL_sound ${PHYSFS_LIBRARIES}) endif()
