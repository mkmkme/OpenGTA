add_library(ogta_util_ OBJECT
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
    set.cpp
    string_helpers.cpp
    timer.cpp)
target_include_directories(ogta_util_ PUBLIC
    ${OpenGTA_SOURCE_DIR}/src
    ${OpenGTA_BINARY_DIR}/src)
target_link_libraries(ogta_util_ PUBLIC ogta::opensteer ogta::math SDL2::SDL2)

add_library(ogta::util ALIAS ogta_util_)

# on GCC 13 (>= 13, < 14), link with -lstdc++_libbacktrace
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC version: ${CMAKE_CXX_COMPILER_VERSION}")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0)
        message(STATUS "Using stdc++exp for GCC version ${CMAKE_CXX_COMPILER_VERSION}")
        target_link_libraries(ogta_util_ PUBLIC stdc++exp)
    elseif (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.1)
        message(STATUS "Using stdc++_libbacktrace for GCC version ${CMAKE_CXX_COMPILER_VERSION}")
        target_link_libraries(ogta_util_ PUBLIC stdc++_libbacktrace)
    else()
        message(FATAL_ERROR "GCC version ${CMAKE_CXX_COMPILER_VERSION} is not supported for linking with stdc++_libbacktrace. Please use GCC 13.1 or later.")
    endif()
endif()
