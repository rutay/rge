cmake_minimum_required(VERSION 3.16)

# This CMakeLists.txt should only be used during development.
# It basically adds as subdirectories all the examples and potentially tests.

project(rge)

set(RGE_HOME ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "" FORCE)
include("${RGE_HOME}/RGECommon.cmake")

# ----------------------------------------------------------------
# Examples
# ----------------------------------------------------------------

set(EXAMPLES_DIR "examples")

add_subdirectory("${EXAMPLES_DIR}/simple_game")
