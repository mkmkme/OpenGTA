set(OGTA_GRAPHICS_COMMON_SOURCES
    base.cpp
    camera.cpp
    cityview.cpp
    font.cpp
    frustum.cpp
    screen.cpp
    spritecache.cpp
    texturecache.cpp)

add_library(graphics_legacy OBJECT ${OGTA_GRAPHICS_COMMON_SOURCES})
target_link_libraries(graphics_legacy PUBLIC ogta::math SDL2::SDL2 glad::glad)
target_include_directories(graphics_legacy PRIVATE ${OpenGTA_SOURCE_DIR}/src
                                                    ${OpenGTA_BINARY_DIR}/src)

add_library(ogta::graphics ALIAS graphics_legacy)

if(OGTA_USE_MODERN_GL)
  add_library(
    graphics_modern OBJECT
    ${OGTA_GRAPHICS_COMMON_SOURCES}
    shader.cpp
    sprite-renderer-modern.cpp
    font-renderer-modern.cpp)
  target_link_libraries(graphics_modern PUBLIC ogta::math SDL2::SDL2 glad::glad)
  target_include_directories(graphics_modern PRIVATE ${OpenGTA_SOURCE_DIR}/src
                                                      ${OpenGTA_BINARY_DIR}/src)
  target_compile_definitions(graphics_modern PUBLIC OGTA_USE_MODERN_GL)

  add_library(ogta::graphics_modern ALIAS graphics_modern)
endif()
