cmake_minimum_required(VERSION 3.19)
project(bbb.osc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- load bbb_add_external() ---
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/bbb_external.cmake)

# --- output directory ---
set(C74_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/externals)

# --- shared include paths ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)

# --- version generation ---
string(MAKE_C_IDENTIFIER "${PROJECT_NAME}" BBB_PACKAGE_NAME_UPPER)
string(TOUPPER "${BBB_PACKAGE_NAME_UPPER}" BBB_PACKAGE_NAME_UPPER)
set(BBB_VERSION_MACRO "${BBB_PACKAGE_NAME_UPPER}_VERSION")
add_custom_target(generate_version ALL
    COMMAND "${CMAKE_COMMAND}"
        -DWORKING_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        -DVERSION_FILE="${CMAKE_CURRENT_SOURCE_DIR}/source/bbb/version.h"
        -DVERSION_MACRO="${BBB_VERSION_MACRO}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.cmake"
)

# --- auto-discover external projects ---
MACRO(SUBDIRLIST result curdir)
	FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
	SET(dirlist "")
	FOREACH(child ${children})
		IF(IS_DIRECTORY ${curdir}/${child})
			LIST(APPEND dirlist ${child})
		ENDIF()
	ENDFOREACH()
	SET(${result} ${dirlist})
ENDMACRO()

SUBDIRLIST(PROJECT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/source/projects)
foreach(project_dir ${PROJECT_DIRS})
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir}/CMakeLists.txt")
		message("Generating: ${project_dir}")
		add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/source/projects/${project_dir})
	endif()
endforeach()

option(BBB_OSC_BUILD_TESTS "Build bbb.osc tests" OFF)

if(BBB_OSC_BUILD_TESTS)
	enable_testing()
	set(CMAKE_EXE_LINKER_FLAGS "")
	foreach(config DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
		set(CMAKE_EXE_LINKER_FLAGS_${config} "")
	endforeach()
	add_executable(bbb_osc_path_tests tests/osc_path_tests.cpp)
	target_include_directories(bbb_osc_path_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)
	add_test(NAME bbb_osc_path_tests COMMAND bbb_osc_path_tests)
endif()

set(CMAKE_SUPPRESS_REGENERATION true)
