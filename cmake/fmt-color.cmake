# For some reason, currently fmt/color.h is broken for this repo with Clang,
# let's disable it for Clang for now.
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(OGTA_USE_FMT_COLOR ON)
endif()
