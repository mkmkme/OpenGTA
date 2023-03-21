# MSVC supports __VA_OPT__ only with /Zc:preprocessor option.
if(MSVC)
   add_compile_options(/Zc:preprocessor)
endif()

# For GCC and Clang, add the -fdiagnostics-color=always option to force colored output.
if(${FORCE_COLORED_OUTPUT})
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options(-fdiagnostics-color=always)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options(-fcolor-diagnostics)
    endif()
endif()
