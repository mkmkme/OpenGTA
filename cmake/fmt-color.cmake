# For some reason, currently fmt/color.h is broken for this repo with Clang,
# let's enable it only for GCC.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(OGTA_USE_FMT_COLOR ON)
endif()
