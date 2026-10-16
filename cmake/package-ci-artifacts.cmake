cmake_minimum_required(VERSION 3.21)

get_filename_component(source_dir "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
set(build_dir "${source_dir}/build")
set(package_dir "${build_dir}/artifact")
file(MAKE_DIRECTORY "${package_dir}")

if(CMAKE_HOST_WIN32)
  set(executable_suffix ".exe")
else()
  set(executable_suffix "")
endif()

if(CMAKE_HOST_UNIX)
  file(WRITE "${package_dir}/runtime-dependencies.txt" "")
endif()

foreach(program viewer spriteplayer gfxextract)
  set(executable "${build_dir}/programs/${program}${executable_suffix}")
  file(COPY "${executable}" DESTINATION "${package_dir}")

  if(CMAKE_HOST_APPLE)
    execute_process(COMMAND otool -L "${executable}"
                    OUTPUT_VARIABLE dependencies COMMAND_ERROR_IS_FATAL ANY)
  elseif(CMAKE_HOST_UNIX)
    execute_process(COMMAND ldd "${executable}"
                    OUTPUT_VARIABLE dependencies COMMAND_ERROR_IS_FATAL ANY)
  endif()
  if(CMAKE_HOST_UNIX)
    file(APPEND "${package_dir}/runtime-dependencies.txt" "${dependencies}\n")
  endif()
endforeach()

if(CMAKE_HOST_WIN32)
  file(GLOB runtime_dlls "${build_dir}/programs/*.dll")
  if(NOT runtime_dlls)
    message(FATAL_ERROR "No app-local runtime DLLs found in build/programs")
  endif()
  file(COPY ${runtime_dlls} DESTINATION "${package_dir}")
endif()

file(COPY "${source_dir}/license.txt" "${source_dir}/licenses"
     DESTINATION "${package_dir}")
file(GLOB dependency_licenses "${build_dir}/vcpkg_installed/*/share/*/copyright")
foreach(license IN LISTS dependency_licenses)
  get_filename_component(port_dir "${license}" DIRECTORY)
  get_filename_component(port "${port_dir}" NAME)
  file(MAKE_DIRECTORY "${package_dir}/licenses/vcpkg/${port}")
  file(COPY "${license}" DESTINATION "${package_dir}/licenses/vcpkg/${port}")
endforeach()
