add_library(glad_ STATIC ${CMAKE_CURRENT_LIST_DIR}/src/gl.c)
target_include_directories(glad_ PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(glad::glad ALIAS glad_)
