# Coldet
add_library(coldet_3d_ STATIC coldet-3d/coldet/math3d.cpp)
target_compile_definitions(coldet_3d_ PUBLIC GCC)
target_include_directories(coldet_3d_
    PUBLIC
        ${OpenGTA_SOURCE_DIR}/third_party/coldet-3d
    PRIVATE
        ${OpenGTA_BINARY_DIR}/third_party/coldet-3d/coldet)
add_library(ogta::coldet-3d ALIAS coldet_3d_)

# Math by Miguel Gomez
add_library(gomez_math_ STATIC
    gomez-math/math/obox.cpp
    gomez-math/math/plane.cpp)
target_include_directories(gomez_math_
    PUBLIC
        ${OpenGTA_SOURCE_DIR}/third_party/gomez-math
    PRIVATE
        ${OpenGTA_BINARY_DIR}/third_party/gomez-math/math)
target_link_libraries(gomez_math_ PUBLIC ogta::coldet-3d)
add_library(ogta::math ALIAS gomez_math_)

# OpenSteer
add_library(opensteer_ STATIC opensteer/src/Clock.cpp)
target_include_directories(opensteer_ PUBLIC
    ${OpenGTA_SOURCE_DIR}/third_party/opensteer/include)
add_library(ogta::opensteer ALIAS opensteer_)
