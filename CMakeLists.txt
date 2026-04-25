cmake_minimum_required(VERSION 3.19)
project(bbb.osc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_custom_target(generate_version ALL
    COMMAND ${CMAKE_COMMAND}
        -DWORKING_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -DVERSION_FILE=${CMAKE_CURRENT_SOURCE_DIR}/source/bbb/version.h
        -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.cmake
)

add_subdirectory(source/projects/bbb.osc.send)
add_subdirectory(source/projects/bbb.osc.receive)
