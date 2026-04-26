# source/projects/bbb.osc.receive/CMakeLists.txt

set(BBB_OSC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../deps/bbb-osc")

if(WIN32)
    set(PLATFORM_DEPS "")
else()
    set(PLATFORM_DEPS pthread)
endif()

bbb_add_external(
    DEPS ${PLATFORM_DEPS}
    INCLUDES
        ${BBB_OSC_DIR}/include
        ${BBB_OSC_DIR}/third-party/asio/include
        ${BBB_OSC_DIR}/third-party/bit_by_bit/include
        ${BBB_OSC_DIR}/third-party/oscpp/include
)
