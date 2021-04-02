cmake_minimum_required(VERSION 3.10)

project(simple_game)

set(RGE_HOME "${CMAKE_CURRENT_SOURCE_DIR}/../.." CACHE STRING "" FORCE)
include("${RGE_HOME}/RGECommon.cmake")

set(SRCS
	"src/entry.cpp"
	"src/entry.hpp"
)

rge_begin_game(simple_game ${SRCS})

rge_def_model("McLaren" "assets/models/McLaren.glb")
rge_req_model("McLaren")

rge_def_model("GumballAndDarwin" "assets/models/gumball_darwin.glb")
rge_req_model("GumballAndDarwin")

rge_end_game()
