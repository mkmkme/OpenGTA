add_library(ogta-util STATIC
    animation.cpp
    cell_iterator.cpp
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
target_link_libraries(ogta-util ogta-lua opensteer-clock fmt::fmt physfs-static SDL2::SDL2)
