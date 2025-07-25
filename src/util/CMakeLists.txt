add_library(
  ogta_util_ OBJECT
  animation.cpp
  cell_iterator.cpp
  errors.cpp
  file-manager.cpp
  file_helper.cpp
  gui.cpp
  image_loader.cpp
  key_handler.cpp
  log.cpp
  map_helper.cpp
  pf_tree.cpp
  physfsrwops.c
  set.cpp
  string_helpers.cpp
  timer.cpp)
target_include_directories(ogta_util_ PUBLIC ${OpenGTA_SOURCE_DIR}/src
                                             ${OpenGTA_BINARY_DIR}/src)
target_link_libraries(
  ogta_util_
  PUBLIC
    ogta::core
    ogta::opensteer
    ogta::math
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
)

add_library(ogta::util ALIAS ogta_util_)

if(OGTA_STD_STACKTRACE_AVAILABLE)
  target_link_libraries(ogta_util_ PUBLIC ${OGTA_STD_STACKTRACE_LIB})
endif()
