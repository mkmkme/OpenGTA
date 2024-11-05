add_library(graphics_ OBJECT
    base.cpp
    camera.cpp
    cityview.cpp
    font.cpp
    frustum.cpp
    screen.cpp
    spritecache.cpp
    texturecache.cpp)
target_link_libraries(graphics_ PUBLIC ogta::math)
target_include_directories(graphics_ PRIVATE
    ${OpenGTA_SOURCE_DIR}/src
    ${OpenGTA_BINARY_DIR}/src)

add_library(ogta::graphics ALIAS graphics_)
