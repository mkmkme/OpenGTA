add_library(
  graphics_ OBJECT
  base.cpp
  camera.cpp
  cityview.cpp
  font.cpp
  frustum.cpp
  screen.cpp
  spritecache.cpp
  texturecache.cpp)
target_link_libraries(graphics_ PUBLIC ogta::math SDL2::SDL2 glad::glad)
target_include_directories(graphics_ PRIVATE ${OpenGTA_SOURCE_DIR}/src
                                             ${OpenGTA_BINARY_DIR}/src)

add_library(ogta::graphics ALIAS graphics_)

if(OGTA_USE_MODERN_GL)
  target_sources(graphics_ PRIVATE shader.cpp sprite-renderer-modern.cpp font-renderer-modern.cpp)
  target_compile_definitions(graphics_ PUBLIC OGTA_USE_MODERN_GL)
endif()
