cmake_minimum_required(VERSION 3.19)

if(NOT DEFINED C74_MIN_API_DIR)
    set(C74_MIN_API_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/min-api")
endif()

project(bbb.osc.receive)

set(SOURCE_FILES
    ${PROJECT_NAME}.cpp
)

include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)

add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${C74_INCLUDES})

set(BBB_OSC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/bbb-osc")
target_include_directories(${PROJECT_NAME} PRIVATE
    ${BBB_OSC_DIR}/include
    ${BBB_OSC_DIR}/third-party/asio/include
    ${BBB_OSC_DIR}/third-party/bit_by_bit/include
    ${BBB_OSC_DIR}/third-party/oscpp/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE pthread)

set(HELP_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}.maxhelp")
set(HELP_DEST "${CMAKE_CURRENT_SOURCE_DIR}/../../../help/${PROJECT_NAME}.maxhelp")
if(EXISTS "${HELP_SOURCE}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${HELP_SOURCE}" "${HELP_DEST}"
    )
endif()

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
