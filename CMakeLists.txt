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

find_package(cxxopts REQUIRED)
find_package(fmt REQUIRED)
find_package(glm REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image)
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
