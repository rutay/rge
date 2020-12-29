cmake_minimum_required(VERSION 3.10)

project(simple_game)

set(RGE_HOME "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE STRING "" FORCE)
include("${RGE_HOME}/RGECommon.cmake")

set(SRCS
	"src/entry.cpp"
	"src/entry.hpp"
)

set(SHADERS
	# ...
)

rge_define_game(simple_game ${SRCS} ${SHADERS})

rge_require_assets(
	simple_game
	"models/gumball_darwin.glb"
	"models/shrek"
	"models/McLaren.glb"
)
