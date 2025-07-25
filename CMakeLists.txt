cmake_minimum_required(VERSION 3.21)

project(OpenGTA)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/build-options.cmake)
include(cmake/colored-output.cmake)
include(cmake/fmt-color.cmake)
include(cmake/msvc-force-utf8.cmake)
include(cmake/sanitize.cmake)
include(cmake/std-stacktrace.cmake)

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
