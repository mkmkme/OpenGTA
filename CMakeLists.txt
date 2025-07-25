cmake_minimum_required(VERSION 3.21)

project(OpenGTA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CMakeOptions.txt)
include(CompilerOptions.cmake)

# With MSVC, we need to force UTF-8 because this is required by fmtlib
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

# Enable diagnostics colors for clang/gcc
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options("-fdiagnostics-color=always")
endif()

# if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#     add_compile_options("-stdlib=libc++")
#     add_link_options("-stdlib=libc++")
# endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(OGTA_USE_FMT_COLOR ON)
endif()

option(SANITIZE "Enable one of the sanitizers" "")
if (SANITIZE)
    if (SANITIZE STREQUAL "address")
        set(ASAN_FLAGS "-fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
    elseif (SANITIZE STREQUAL "memory")
        set(MSAN_FLAGS "-fsanitize=memory -fsanitize-memory-use-after-dtor -fsanitize-memory-track-origins -fno-optimize-sibling-calls")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSAN_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MSAN_FLAGS}")
    elseif (SANITIZE STREQUAL "undefined")
        set(UBSAN_FLAGS "-fsanitize=undefined -fno-sanitize-recover=all -fno-sanitize=float-divide-by-zero")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${UBSAN_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${UBSAN_FLAGS}")
    endif()
endif()

set(OGTA_STD_STACKTRACE_AVAILABLE OFF)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(OGTA_STD_STACKTRACE_AVAILABLE ON)
endif()

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image)
if(SDL2_image_FOUND)
    set(OGTA_WITH_SDL_IMAGE ON)
endif()
find_package(OpenGL REQUIRED)
find_package(PhysFS REQUIRED)
find_package(Lua REQUIRED)
find_package(GTest REQUIRED)

set(OpenGTA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(OpenGTA_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(programs)
add_subdirectory(tests)
add_subdirectory(tools)
