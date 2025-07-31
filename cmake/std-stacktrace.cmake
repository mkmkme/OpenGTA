set(OGTA_STD_STACKTRACE_AVAILABLE OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|MSVC")
  set(OGTA_STD_STACKTRACE_AVAILABLE ON)
endif()

# on GCC 13 (>= 13, < 14), link with -lstdc++_libbacktrace
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "GCC version: ${CMAKE_CXX_COMPILER_VERSION}")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14.0)
    message(
      STATUS "Using stdc++exp for GCC version ${CMAKE_CXX_COMPILER_VERSION}")
    set(OGTA_STD_STACKTRACE_LIB stdc++exp)
  elseif(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.1)
    message(
      STATUS
        "Using stdc++_libbacktrace for GCC version ${CMAKE_CXX_COMPILER_VERSION}"
    )
    set(OGTA_STD_STACKTRACE_LIB stdc++_libbacktrace)
  else()
    message(
      FATAL_ERROR
        "GCC version ${CMAKE_CXX_COMPILER_VERSION} is not supported for linking with stdc++_libbacktrace. Please use GCC 13.1 or later."
    )
  endif()
endif()
