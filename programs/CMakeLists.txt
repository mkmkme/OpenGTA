add_executable(spriteplayer spriteplayer/main.cpp)
target_include_directories(spriteplayer PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(
  spriteplayer PRIVATE ogta::core ogta::graphics ogta::util OpenGL::GLU
                       fmt::fmt ${PHYSFS_LIBRARY})

add_executable(
  viewer viewer/helpers.cpp viewer/main.cpp viewer/version-info.cpp
         viewer/viewer.cpp viewer/viewer-config.cpp)
target_include_directories(viewer PRIVATE ${PHYSFS_INCLUDE_DIR})
target_link_libraries(
  viewer
  PRIVATE ogta::core
          ogta::graphics
          ogta::lua-addon
          ogta::math
          ogta::opensteer
          ogta::util
          fmt::fmt
          OpenGL::GLU
          ${PHYSFS_LIBRARY})

add_executable(gfxextract gfx_extract/main.cpp)
target_include_directories(
  gfxextract PRIVATE ${PHYSFS_INCLUDE_DIR} ${OpenGTA_SOURCE_DIR}/src
                     ${OpenGTA_BINARY_DIR}/src)
target_link_libraries(gfxextract PRIVATE ogta::core ogta::graphics ogta::util
                                         fmt::fmt OpenGL::GLU ${PHYSFS_LIBRARY})
