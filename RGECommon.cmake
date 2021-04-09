
include_guard()

message("RGE_HOME: ${RGE_HOME}")

find_package(Python3 COMPONENTS Interpreter)
if (NOT Python3_FOUND)
	message(FATAL_ERROR "System python3 not found. Can't go on.")
endif()

set(SYSTEM_PYTHON3 ${Python3_EXECUTABLE})

if (WIN32)
	set(VENV_ACTIVATE "${RGE_HOME}/venv/Scripts/activate.bat")
	set(VENV_PY "${RGE_HOME}/venv/Scripts/python.exe")
	set(VENV_PIP "${RGE_HOME}/venv/Scripts/pip.exe")
else()
	set(VENV_ACTIVATE "${RGE_HOME}/venv/bin/activate")
	set(VENV_PY "${RGE_HOME}/venv/bin/python")
	set(VENV_PIP "${RGE_HOME}/venv/bin/pip")
endif()

if (NOT EXISTS ${VENV_ACTIVATE})
	message("Creating venv...")
	execute_process(
			COMMAND ${SYSTEM_PYTHON3} -m venv venv
			WORKING_DIRECTORY ${RGE_HOME}
	)

	message("Installing pip requirements...")
	execute_process(
			COMMAND ${VENV_PIP} install -r requirements.txt
			WORKING_DIRECTORY ${RGE_HOME}
	)
	# todo install from requirements.txt
endif()

set(OpenGL_GL_PREFERENCE "LEGACY")

# ------------------------------------------------------------------------------------------------
# Options
# ------------------------------------------------------------------------------------------------

set(PLATFORM_TRIPLET "ALL_SDL2_GL")

# ------------------------------------------------------------------------------------------------
# Utilities
# ------------------------------------------------------------------------------------------------

function (rge_abs_path ABS_PATHS_VAR REL_PATHS)
	unset(${ABS_PATHS_VAR} PARENT_SCOPE)
	foreach (REL_PATH ${REL_PATHS})
		list(APPEND ${ABS_PATHS_VAR} "${RGE_HOME}/${REL_PATH}")
	endforeach()
	set(${ABS_PATHS_VAR} ${${ABS_PATHS_VAR}} PARENT_SCOPE)
endfunction()

function (rge_log MODE MSG)
	message(${MODE} "[RGE] ${MSG}")
endfunction()


# ------------------------------------------------------------------------------------------------
# RGE initialization
# ------------------------------------------------------------------------------------------------

rge_log(STATUS "Initialization")

set(CMAKE_CXX_STANDARD 20)

if (NOT EMSCRIPTEN) # If it's not emscripten, then it's desktop.
	set(IS_HOST_PLATFORM TRUE)
endif()

set(BUILD_CLIENT TRUE)
add_compile_definitions(BUILD_CLIENT)

set(RGE_SRC
	"src/server/main.cpp"

	"src/rge_init.hpp"

	"src/cli/platform/platform.hpp"
	"src/cli/renderer/renderer.cpp"
	"src/cli/renderer/renderer.hpp"

	"src/cli/platform/platform_all_sdl2_gl.cpp" # TODO choose platform and renderer dynamically.
	"src/cli/renderer/renderer_gl.cpp"
	"src/cli/renderer/renderer_gl.hpp"
	"src/cli/renderer/material_serializer.hpp"
	"src/cli/renderer/material_serializers_def.hpp"

	"src/resources/filesystem_resource_provider.cpp" # todo dynamically choose which resource provider to use
	"src/resources/resource_provider.hpp"
	"src/resources/resource_manager.hpp"
	"src/resources/resource.hpp"

	"src/cli/camera.hpp"
	"src/cli/camera.cpp"


	"src/scene/materials_def.hpp"

	# Game
	"src/cli/game.hpp"
	"src/cli/game.cpp"

	# Scene
	"src/scene/scene.hpp"
	"src/scene/scene.cpp"
	"src/scene/material.hpp"
    "src/scene/tinygltf_scene_loader.hpp"
	"src/scene/tinygltf_scene_loader.cpp"
	"src/scene/utils.hpp"
	"src/scene/utils.cpp"

	"src/packet.hpp"

	"src/util.cpp"
	"src/util.hpp"
)

set(RGE_INCLUDE_DIR "src")

if (EMSCRIPTEN)
	set(INITIAL_MEMORY 52428800)  # 50MB of memory just for your game ^^
	set(TOTAL_MEMORY 67108864)    # 64MB of total memory

	set(CMAKE_BUILD_TYPE Release)

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s WASM=1 -s TOTAL_MEMORY=${TOTAL_MEMORY} -s ALLOW_MEMORY_GROWTH=0")
endif()

# ------------------------------------------------------------------------------------------------
# Tools
# ------------------------------------------------------------------------------------------------

set(RGE_TOOLS_DIR "${RGE_HOME}/tools")

set(RGE_SHADERC "${RGE_TOOLS_DIR}/shaderc.py")

# ------------------------------------------------------------------------------------------------
# Third party
# ------------------------------------------------------------------------------------------------

rge_log(STATUS "Loading third-part dependencies")

set(RGE_THIRD_PARTY_DIR "third_party")

# ----------------------------------------------------------------
# glad
# ----------------------------------------------------------------

set(GLAD_DIR "${RGE_THIRD_PARTY_DIR}/glad")
add_subdirectory("${RGE_HOME}/${GLAD_DIR}" ${GLAD_DIR})

# ----------------------------------------------------------------
# Bullet3
# ----------------------------------------------------------------

rge_log(STATUS "Loading Bullet3")

set(USE_DOUBLE_PRECISION OFF CACHE BOOL "" FORCE)
set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)

set(BULLET_DIR "${RGE_THIRD_PARTY_DIR}/bullet3")

add_subdirectory("${RGE_HOME}/${BULLET_DIR}" ${BULLET_DIR})

set(BULLET_INCLUDE_DIR "${BULLET_DIR}/src")
set(BULLET_LIBS LinearMath Bullet3Common BulletInverseDynamics BulletCollision BulletDynamics BulletSoftBody)

# ----------------------------------------------------------------
# bgfx
# ----------------------------------------------------------------

rge_log(STATUS "Loading BGFX")

set(BGFX_CMAKE_DIR "${RGE_THIRD_PARTY_DIR}/bgfx.cmake")

if (IS_HOST_PLATFORM)
	set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)  # Build BGFX tools (like shaderc, geometryc etc.) only if we're on a desktop platform.
else()
	set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
endif()
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL        OFF CACHE BOOL "" FORCE)
set(BGFX_CUSTOM_TARGETS OFF CACHE BOOL "" FORCE)
set(BGFX_CONFIG_DEBUG   ON  CACHE BOOL "" FORCE)

add_subdirectory("${RGE_HOME}/${BGFX_CMAKE_DIR}" ${BGFX_CMAKE_DIR})

set(BGFX_INCLUDE_DIR        "${BGFX_CMAKE_DIR}/bgfx/include")
set(BGFX_SHADER_INCLUDE_DIR "${BGFX_CMAKE_DIR}/bgfx/src")
set(BX_INCLUDE_DIR          "${BGFX_CMAKE_DIR}/bx/include")
set(BIMG_INCLUDE_DIR        "${BGFX_CMAKE_DIR}/bimg/include")
set(BGFX_INCLUDE_DIRS "${BGFX_INCLUDE_DIR} ${BGFX_SHADER_INCLUDE_DIR} ${BX_INCLUDE_DIR} ${BIMG_INCLUDE_DIR}")
set(BGFX_LIBS bgfx)

# ----------------------------------------------------------------
# tinygltf
# ----------------------------------------------------------------

rge_log(STATUS "Loading TinyGLTF")

set(TINYGLTF_DIR "${RGE_THIRD_PARTY_DIR}/tinygltf")

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_GL_EXAMPLES  OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE  OFF CACHE BOOL "" FORCE)

add_subdirectory("${RGE_HOME}/${TINYGLTF_DIR}" ${TINYGLTF_DIR})

set(TINYGLTF_INCLUDE_DIR ${TINYGLTF_DIR})

# ----------------------------------------------------------------
# SDL2
# ----------------------------------------------------------------

rge_log(STATUS "Loading SDL2")

set(SDL2_DIR "${RGE_THIRD_PARTY_DIR}/sdl2")

add_subdirectory("${RGE_HOME}/${SDL2_DIR}" ${SDL2_DIR})

set(SDL2_INCLUDE_DIR "${SDL2_DIR}/include")
set(SDL2_LIBS SDL2main SDL2-static)

# ------------------------------------------------------------------------------------------------
# Game
# ------------------------------------------------------------------------------------------------

# `rge_define_game` <GAME_NAME>
#
# * GAME_NAME    - the name of the game executable created.
function (rge_define_game GAME_NAME SRCS)
	rge_log(NOTICE "Game definition: ${GAME_NAME}")

	# Prepares a list containing the RGE's sources and the game's sources.
	# The engine sources must be taken according to the set RGE_HOME.
	rge_abs_path(FINAL_SRCS "${RGE_SRC}")
	list(APPEND FINAL_SRCS ${SRCS})

	add_executable(${GAME_NAME} ${FINAL_SRCS})

	# Python configuration

	set(ENV{RGE_HOME} "${RGE_HOME}")
	set(ENV{GAME_SRC_DIR} "${CMAKE_CURRENT_SOURCE_DIR}")
	#set(ENV{GAME_BIN_DIR} "$<TARGET_FILE_DIR:${GAME_NAME}>") We don't know the binary path during configuration-time!

	set(BUILD_CFG_CMD ${VENV_PY} ${RGE_HOME}/build_cfg/cfg_exec.py -c ${RGE_HOME}/base_cfg.json ${CMAKE_CURRENT_SOURCE_DIR}/rge_cfg.json)

	execute_process(
		COMMAND ${BUILD_CFG_CMD} -o "on_cmake_configure"
		WORKING_DIRECTORY "${RGE_HOME}"
	)

	add_custom_command(
		TARGET ${GAME_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E env
				RGE_HOME=${RGE_HOME}
				GAME_SRC_DIR=${CMAKE_CURRENT_SOURCE_DIR}
				GAME_BIN_DIR=$<TARGET_FILE_DIR:${GAME_NAME}>

				${BUILD_CFG_CMD} -o on_post_build
		COMMENT "[RGE] Executing post-build configuration script..."
	)

	# ----------------------------------------------------------------
	# Dependencies
	# ----------------------------------------------------------------

	# RGE
	target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${RGE_INCLUDE_DIR}")

	target_include_directories(${GAME_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.gen_cfg")

	# GLAD
	# target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${GLAD_INCLUDE_DIR}")
	target_link_libraries(${GAME_NAME} glad)

	# Bullet3
	target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${BULLET_INCLUDE_DIR}")
	target_link_libraries(${GAME_NAME} ${BULLET_LIBS})

	target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${TINYGLTF_INCLUDE_DIR}")

	if (BUILD_CLIENT)
		# bgfx
		rge_abs_path(BGFX_INCLUDE_DIRS_ABS "${BGFX_INCLUDE_DIRS}")

		target_include_directories(${GAME_NAME} PUBLIC ${BGFX_INCLUDE_DIRS_ABS})
		target_link_libraries(${GAME_NAME} ${BGFX_LIBS})

		# SDL2
		target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${SDL2_INCLUDE_DIR}")
		target_link_libraries(${GAME_NAME} ${SDL2_LIBS})

	endif()
endfunction()

