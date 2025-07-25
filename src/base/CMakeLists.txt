execute_process(
  COMMAND ${CMAKE_SOURCE_DIR}/scripts/get-version.sh
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE OGTA_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "OpenGTA version: ${OGTA_VERSION}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)
