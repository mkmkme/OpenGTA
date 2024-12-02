# Math by Miguel Gomez
add_library(gomez_math_ STATIC
    gomez-math/math/obox.cpp
    gomez-math/math/plane.cpp)
target_include_directories(gomez_math_
    PUBLIC
        ${OpenGTA_SOURCE_DIR}/third_party/gomez-math
    PRIVATE
        ${OpenGTA_BINARY_DIR}/third_party/gomez-math/math)
target_link_libraries(gomez_math_ PUBLIC glm::glm)
add_library(ogta::math ALIAS gomez_math_)

# OpenSteer
add_library(opensteer_ STATIC opensteer/src/Clock.cpp)
target_include_directories(opensteer_ PUBLIC
    ${OpenGTA_SOURCE_DIR}/third_party/opensteer/include)
add_library(ogta::opensteer ALIAS opensteer_)

# glad
add_library(glad_ STATIC glad/src/gl.c)
target_include_directories(glad_ PUBLIC
    ${OpenGTA_SOURCE_DIR}/third_party/glad/include)
add_library(ogta::glad ALIAS glad_)
