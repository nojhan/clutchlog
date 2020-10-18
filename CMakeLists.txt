
######################################################################################
# Project settings
######################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project("clutchlog"
    VERSION 0.5.0
    DESCRIPTION "A logging system which targets versatile debugging")

enable_language(CXX) # C++
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_LIBRARIES -lstdc++fs)

######################################################################################
# Configurable user settings
######################################################################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# put binaries in the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Dump used compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(WITH_CLUTCHLOG "Define WITH_CLUTCHLOG, whatever the build type." OFF)
if(WITH_CLUTCHLOG)
    add_definitions(-DWITH_CLUTCHLOG)
endif()

# Do not build documentation by default.
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" OFF)

######################################################################################
# Start building
######################################################################################

# Doxygen (iff documentation is asked).
if(BUILD_DOCUMENTATION)
    find_package(Doxygen REQUIRED)

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    # ALL allows to build the docs together with any other target.
    add_custom_target( doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
endif()

enable_testing()
add_subdirectory(tests)

