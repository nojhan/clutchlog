
######################################################################################
# Project settings
######################################################################################

cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project("clutchlog")

enable_language(CXX) # C++
set(CMAKE_CXX_STANDARD 17)

## Current version
set(VERSION_MAJOR 0 CACHE STRING "Major version number" )
set(VERSION_MINOR 1 CACHE STRING "Minor version number" )
set(VERSION_PATCH 0 CACHE STRING "Patch version number" )
mark_as_advanced(VERSION_MAJOR VERSION_MINOR VERSION_PATCH)

# There's no point building clutchlog tests in Release
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD_LIBRARIES -lstdc++fs)

######################################################################################
# Configurable user settings
######################################################################################

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# put binaries in the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Dump used compiler flags.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


######################################################################################
# Start building
######################################################################################

enable_testing()
add_subdirectory(tests)

