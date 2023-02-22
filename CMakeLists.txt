cmake_minimum_required(VERSION 3.14)
project(OpenGTA)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL_mixer)
find_package(glu REQUIRED)
find_package(PhysFS REQUIRED)
find_package(lua REQUIRED)


include(compile-options.cmake)
include(cmake/force-compiler-colors.cmake)

configure_file(config.h.in config.h)

include_directories(
    coldet
    math
    opensteer/include
    util
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR})

add_library(opensteer-clock STATIC opensteer/src/Clock.cpp)

add_subdirectory(math)
add_subdirectory(util)

add_library(coldet-math3d STATIC coldet/math3d.cpp)
target_compile_definitions(coldet-math3d PRIVATE GCC)

add_library(ogta STATIC
    ai.cpp
    blockanim.cpp
    blockdata.cpp
    car-info.cpp
    dataholder.cpp
    entity_controller.cpp
    font.cpp
    font_cache.cpp
    game_objects.cpp
    graphics-base.cpp
    graphics-24bit.cpp
    graphics-8bit.cpp
    loaded-anim.cpp
    localplayer.cpp
    map.cpp
    message-db.cpp
    navdata.cpp
    object-info.cpp
    read_ini.cpp
    read_sdt.cpp
    slope_height_func.cpp
    spritemanager.cpp
    train_system.cpp)
target_link_libraries(ogta PUBLIC
    ogta-math
    coldet-math3d
    fmt::fmt
    physfs-static
    SDL2::SDL2
    glu::glu)


add_library(ogta-gl STATIC
    gl_base.cpp
    gl_camera.cpp
    gl_cityview.cpp
    gl_font.cpp
    gl_frustum.cpp
    gl_screen.cpp
    gl_spritecache.cpp
    gl_texturecache.cpp
    util/physfsrwops.c)
target_link_libraries(ogta-gl ogta)

add_executable(viewer
    main2.cpp
    viewer.cpp)

target_link_libraries(viewer
    ogta
    ogta-gl
    ogta-util
    opensteer-clock
    cxxopts::cxxopts)

add_executable(gfxextract EXCLUDE_FROM_ALL
    gfx_extract.cpp
    graphics-24bit.cpp
    graphics-8bit.cpp
    map.cpp
    navdata.cpp
    dataholder.cpp)

# TODO: refactor sources so this is not needed
target_link_options(gfxextract PRIVATE
    -Xlinker --unresolved-symbols
    -Xlinker ignore-all)

target_link_libraries(gfxextract
    ogta-util
    SDL2::SDL2
    glu::glu
    physfs-static)

if(lua_FOUND)
    add_subdirectory(lua_addon)
    include_directories(lua_addon)
    add_executable(luaviewer EXCLUDE_FROM_ALL
        main2.cpp
        viewer.cpp)
    target_compile_definitions(luaviewer PRIVATE WITH_LUA)
    target_link_libraries(luaviewer
        ogta
        ogta-gl
        ogta-lua
        ogta-util
        opensteer-clock
        cxxopts::cxxopts)

    add_executable(lua_map_test EXCLUDE_FROM_ALL
        main2.cpp
        tests/lua_map_test.cpp)

    target_link_libraries(lua_map_test
        ogta
        ogta-gl
        ogta-util
        ogta-lua
        physfs-static
        SDL2::SDL2)
else()
    message(WARNING "Lua not found! Will build without lua support")
endif()

add_executable(spriteplayer EXCLUDE_FROM_ALL tools/spriteplayer.cpp)

target_link_libraries(spriteplayer
    ogta
    ogta-gl
    ogta-util
    opensteer-clock
    SDL2::SDL2
    SDL2::SDL2main
    glu::glu
    physfs-static)

add_executable(slopeview EXCLUDE_FROM_ALL
    main.cpp
    tools/display_slopes.cpp
    navdata.cpp
    map.cpp
    common_sdl_gl.cpp)

target_link_libraries(slopeview
    ogta
    ogta-util
    SDL2::SDL2
    glu::glu
    physfs-static)

# broken due to missing slope2_data.h
add_executable(blockview EXCLUDE_FROM_ALL
    main2.cpp
    tools/blockview.cpp)

target_link_libraries(blockview
    SDL2::SDL2
    physfs-static
    glu::glu)

add_executable(g24-dumper EXCLUDE_FROM_ALL tools/g24-dumper.cpp)

target_link_libraries(g24-dumper
    ogta
    ogta-util
    SDL2::SDL2
    SDL2::SDL2main
    physfs-static)

add_executable(display-font EXCLUDE_FROM_ALL tools/display-font.cpp)

target_link_libraries(display-font
    ogta
    ogta-gl
    ogta-util
    SDL2::SDL2
    SDL2::SDL2main
    physfs-static)

add_executable(obj-dump EXCLUDE_FROM_ALL tools/obj-dump.cpp)

target_link_libraries(obj-dump
    ogta-gl
    ogta-util
    physfs-static
    SDL2::SDL2)

add_executable(car_dump EXCLUDE_FROM_ALL tools/car_dump.cpp)

target_link_libraries(car_dump
    ogta
    ogta-util
    physfs-static
    SDL2::SDL2)

# broken due to missing tests/test_plane.cpp
# add_executable(plane_test
#     tests/test_plane.cpp)

if(SDL_MIXER_FOUND)
    add_executable(sound_test EXCLUDE_FROM_ALL
        util/sound_device.cpp
        util/sound_fx_cache.cpp
        util/sound_music_player.cpp
        util/sound_resample2.cpp
        util/sound_system.cpp
        util/physfsrwops.c
        read_sdt.cpp)

    target_compile_definitions(sound_test PRIVATE SOUND_TEST WITH_SOUND)

    target_link_libraries(sound_test
        ogta-util
        ${SDL2_LIBRARIES}
        ${SDL_MIXER_LIBRARIES}
        # SDL_sound-devel is missing pkg-config file on Fedora 34
        # I would bother adding this myself, but I'm switching to SDL2 anyways
        SDL_sound
        ${PHYSFS_LIBRARIES})
endif()
