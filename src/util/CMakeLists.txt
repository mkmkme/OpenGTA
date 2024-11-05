add_library(ogta_util_ OBJECT
    animation.cpp
    cell_iterator.cpp
    file-manager.cpp
    file_helper.cpp
    gui.cpp
    image_loader.cpp
    key_handler.cpp
    log.cpp
    map_helper.cpp
    pf_tree.cpp
    set.cpp
    string_helpers.cpp
    timer.cpp)
target_include_directories(ogta_util_ PUBLIC
    ${OpenGTA_SOURCE_DIR}/src
    ${OpenGTA_BINARY_DIR}/src)
target_link_libraries(ogta_util_ PUBLIC ogta::opensteer ogta::math)

add_library(ogta::util ALIAS ogta_util_)
