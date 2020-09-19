
######################################################################################
# Project settings
######################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project("clutchlog")

enable_language(CXX) # C++
set(CMAKE_CXX_STANDARD 17)

## Current version
set(VERSION_MAJOR 0 CACHE STRING "Major version number" )
set(VERSION_MINOR 3 CACHE STRING "Minor version number" )
set(VERSION_PATCH 0 CACHE STRING "Patch version number" )
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

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

######################################################################################
# Start building
######################################################################################

enable_testing()
add_subdirectory(tests)

