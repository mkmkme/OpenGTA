execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/get-version.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE OGTA_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "OpenGTA version: ${OGTA_VERSION}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(ogta_core_ OBJECT
    active-map.cpp
    active-style.cpp
    ai.cpp
    blockanim.cpp
    blockdata.cpp
    car-info.cpp
    entity_controller.cpp
    font.cpp
    font_cache.cpp
    game_objects.cpp
    globals.cpp
    graphics-base.cpp
    graphics-24bit.cpp
    graphics-8bit.cpp
    loaded-anim.cpp
    localplayer.cpp
    main-msg-lookup.cpp
    map.cpp
    message-db.cpp
    navdata.cpp
    object-info.cpp
    read_ini.cpp
    read_sdt.cpp
    slope_height_func.cpp
    spritemanager.cpp
    train_system.cpp)
target_include_directories(ogta_core_ PUBLIC
    ${OpenGTA_SOURCE_DIR}/src
    ${OpenGTA_BINARY_DIR}/src)
target_link_libraries(ogta_core_ PUBLIC ogta::opensteer ogta::math SDL2::SDL2)
add_library(ogta::core ALIAS ogta_core_)
