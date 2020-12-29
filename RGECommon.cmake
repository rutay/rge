
if (DEFINED RGE_DEFINED)
	return()
endif()

set(RGE_DEFINED TRUE)

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
	set(USE_GLFW TRUE)
endif()

set(BUILD_CLIENT TRUE)
add_compile_definitions(BUILD_CLIENT)

set(RGE_SRCS
	"src/resource/filesystem_resource_provider.cpp"
	"src/resource/http_resource_provider.cpp"
	"src/resource/resource_provider.cpp"
	"src/resource/resource_provider.hpp"
	"src/cli/boot_glfw.cpp"
	"src/server/main.cpp"
	"src/cli/render/camera.hpp"
	"src/cli/render/camera.cpp"
	"src/cli/render/renderer.cpp"
	"src/cli/render/renderer.hpp"

	# Booter
	"src/cli/boot.hpp"
	"src/cli/boot_html5.cpp"

	# Game
	"src/cli/game.hpp"
	"src/cli/game.cpp"

	# Scene
	"src/scene/scene.hpp"
	"src/scene/scene.cpp"

	"src/scene/gltf_scene_loader.hpp"
	"src/scene/gltf_scene_loader.cpp"
	"src/packet.hpp"
)

set(RGE_INCLUDE_DIR "src")

if (EMSCRIPTEN)
	set(INITIAL_MEMORY 52428800)  # 50MB of memory just for your game ^^
	set(TOTAL_MEMORY 67108864)    # 64MB of total memory

	set(CMAKE_BUILD_TYPE Release)

	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2 -s WASM=1 -s TOTAL_MEMORY=${TOTAL_MEMORY} -s ALLOW_MEMORY_GROWTH=0")
endif()

# ------------------------------------------------------------------------------------------------
# Third party
# ------------------------------------------------------------------------------------------------

rge_log(STATUS "Loading third-part dependencies")

set(RGE_THIRD_PARTY_DIR "third_party")

# Bullet3
# Ref: https://github.com/bulletphysics/bullet3/blob/master/CMakeLists.txt

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

# bgfx
rge_log(STATUS "Loading BGFX")

set(BGFX_CMAKE_DIR "${RGE_THIRD_PARTY_DIR}/bgfx.cmake")

if (IS_HOST_PLATFORM)
	set(BGFX_BUILD_TOOLS ON CACHE BOOL "" FORCE)  # Build BGFX tools (like shaderc, geometryc etc.) only if we're on a desktop platform.
else()
	set(BGFX_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
endif()
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BGFX_INSTALL OFF CACHE BOOL "" FORCE)
set(BGFX_CUSTOM_TARGETS OFF CACHE BOOL "" FORCE)

add_subdirectory("${RGE_HOME}/${BGFX_CMAKE_DIR}" ${BGFX_CMAKE_DIR})

set(BGFX_INCLUDE_DIR        "${BGFX_CMAKE_DIR}/bgfx/include")
set(BGFX_SHADER_INCLUDE_DIR "${BGFX_CMAKE_DIR}/bgfx/src")
set(BX_INCLUDE_DIR          "${BGFX_CMAKE_DIR}/bx/include")
set(BIMG_INCLUDE_DIR        "${BGFX_CMAKE_DIR}/bimg/include")
set(BGFX_INCLUDE_DIRS "${BGFX_INCLUDE_DIR} ${BGFX_SHADER_INCLUDE_DIR} ${BX_INCLUDE_DIR} ${BIMG_INCLUDE_DIR}")
set(BGFX_LIBS bgfx)

# tinygltf
rge_log(STATUS "Loading TinyGLTF")

set(TINYGLTF_DIR "${RGE_THIRD_PARTY_DIR}/tinygltf")

set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_GL_EXAMPLES  OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_VALIDATOR_EXAMPLE  OFF CACHE BOOL "" FORCE)

add_subdirectory("${RGE_HOME}/${TINYGLTF_DIR}" ${TINYGLTF_DIR})

set(TINYGLTF_INCLUDE_DIR ${TINYGLTF_DIR})

# GLFW
# Ref: https://www.glfw.org/docs/3.3/build_guide.html

if (USE_GLFW)
	rge_log(STATUS "Loading GLFW")

	add_compile_definitions(USE_GLFW)

	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

	set(GLFW_DIR "${RGE_THIRD_PARTY_DIR}/glfw")

	add_subdirectory("${RGE_HOME}/${GLFW_DIR}" ${GLFW_DIR})

	set(GLFW_INCLUDE_DIRS "${GLFW_DIR}/include")
	set(GLFW_LIBS glfw)
endif()

# ------------------------------------------------------------------------------------------------
# Assets
# ------------------------------------------------------------------------------------------------

# `rge_compile_shader` <SHADER_FILENAME> <SHADER_TYPE> <SHADER_VARYINGDEF_FILENAME>
#
# * SHADERS_VAR                - the list variable where to append the generated CMake command.
# * SHADER_FILENAME            - where to find the shader, relative to the assets/ folder.
# * SHADER_TYPE                - the type of the shader (v|f).
# * SHADER_VARYINGDEF_FILENAME - where to find the varyingdef,  relative to the assets/ folder.
# * SRC_DIR                    - the directory where to search assets/shaders/${SHADER_NAME}. By default: ${CMAKE_CURRENT_BINARY_DIR}.
# * DST_DIR                    - the directory where to put the assets/shaders/${SHADER_DST_NAME} (compiled).
function (rge_compile_shader SHADERS_VAR SHADER_NAME SHADER_TYPE VARYING_DEF_NAME SRC_DIR DST_DIR)
	if (NOT DEFINED SRC_DIR)
		set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	endif()

	if (NOT DEFINED DST_DIR)
		set(DST_DIR ${CMAKE_CURRENT_BINARY_DIR})
	endif()

	set(ASSETS_SRC_DIR "${SRC_DIR}/assets")
	set(ASSETS_DST_DIR "${DST_DIR}/assets")

	set(SHADERS_SRC_DIR "${ASSETS_SRC_DIR}/cli/shaders")
	set(SHADER_SRC "${SHADERS_SRC_DIR}/${SHADER_NAME}")
	set(VARYING_DEF_SRC "${SHADERS_SRC_DIR}/${VARYING_DEF_NAME}")

	set(SHADERS_DST_DIR "${ASSETS_DST_DIR}/cli/shaders")

	get_filename_component(SHADER_NAME_WLE ${SHADER_NAME} NAME_WLE)
	set(SHADER_DST_NAME "${SHADER_NAME_WLE}.bin")
	set(SHADER_DST "${SHADERS_DST_DIR}/${SHADER_DST_NAME}")

	add_custom_command(
		OUTPUT ${SHADER_DST}
		COMMAND $<TARGET_FILE:shaderc> -f ${SHADER_SRC} -i "${RGE_HOME}/${BGFX_SHADER_INCLUDE_DIR}" --type ${SHADER_TYPE} --varyingdef ${VARYING_DEF_SRC} -o ${SHADER_DST}
		DEPENDS shaderc
		MAIN_DEPENDENCY ${SHADER_SRC} ${VARYING_DEF_SRC}
		COMMENT "Compiling shader using shaderc: ${SHADER_NAME} (${SHADER_TYPE})"
	)

	set(SHADERS_TMP ${${SHADERS_VAR}})
	list(APPEND SHADERS_TMP ${SHADER_DST})
	set(${SHADERS_VAR} ${SHADERS_TMP} PARENT_SCOPE)
endfunction()

# `rge_require_assets` <MODEL_FILENAME>
#
# * MODEL_FILENAME - the filename of the model, relative to the assets/ folder.
function (rge_require_assets GAME_NAME)

	math(EXPR ARGC "${ARGC}-1")
	foreach (ARGi RANGE 1 ${ARGC})

		set(INPUT_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/assets/${ARGV${ARGi}}")
		
		if (IS_DIRECTORY ${INPUT_SRC_PATH})
			file(GLOB_RECURSE ASSETS_SRC_PATH "${INPUT_SRC_PATH}/*")
		else()
			set(ASSETS_SRC_PATH ${INPUT_SRC_PATH})
		endif()

		foreach (ASSET_SRC_PATH ${ASSETS_SRC_PATH})
			
			file(RELATIVE_PATH ASSET_NAME ${CMAKE_CURRENT_SOURCE_DIR} ${ASSET_SRC_PATH})
			set(ASSET_DST_PATH "${CMAKE_CURRENT_BINARY_DIR}/${ASSET_NAME}")

			add_custom_command(
				OUTPUT ${ASSET_DST_PATH}
				COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_SRC_PATH} ${ASSET_DST_PATH}
				MAIN_DEPENDENCY ${ASSET_SRC_PATH}
				COMMENT "Copying required asset: ${ASSET_NAME}"
				VERBATIM
			)

			list(APPEND REQUIRED_ASSETS ${ASSET_DST_PATH})
			
			rge_log(STATUS "Asset: ${ASSET_NAME}")

		endforeach()

	endforeach()
 
	set(REQUIRED_ASSETS_TARGET "${GAME_NAME}_required_assets")
	add_custom_target(${REQUIRED_ASSETS_TARGET} DEPENDS ${REQUIRED_ASSETS})
	add_dependencies(${GAME_NAME} ${REQUIRED_ASSETS_TARGET})

endfunction()

# ------------------------------------------------------------------------------------------------
# Game
# ------------------------------------------------------------------------------------------------

# `rge_define_game` <GAME_NAME>
#
# * GAME_NAME    - the name of the game executable created.
function (rge_define_game GAME_NAME SRCS SHADERS)

	rge_log(NOTICE "Game definition: ${GAME_NAME}")

	# Prepares a list containing the RGE's sources and the game's sources.
	# The engine sources must be taken according to the set RGE_HOME.
	rge_abs_path(FINAL_SRCS "${RGE_SRCS}")
	list(APPEND FINAL_SRCS ${SRCS})

	add_executable(${GAME_NAME} ${FINAL_SRCS})

	# ----------------------------------------------------------------
	# Assets
	# ----------------------------------------------------------------

	rge_compile_shader(SHADERS "draw_mesh_fs.sc" "f" "draw_mesh_def.sc" ${RGE_HOME} "${CMAKE_CURRENT_BINARY_DIR}/.rge")
	rge_compile_shader(SHADERS "simple_inst_vs.sc" "v" "draw_mesh_def.sc" ${RGE_HOME} "${CMAKE_CURRENT_BINARY_DIR}/.rge")

	set(SHADERS_TARGET_NAME "${GAME_NAME}_shaders")
	add_custom_target(${SHADERS_TARGET_NAME} DEPENDS ${SHADERS})
	add_dependencies(${GAME_NAME} ${SHADERS_TARGET_NAME})

	# ----------------------------------------------------------------
	# Dependencies
	# ----------------------------------------------------------------

	# RGE
	target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${RGE_INCLUDE_DIR}")

	# Bullet3
	target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${BULLET_INCLUDE_DIR}")
	target_link_libraries(${GAME_NAME} ${BULLET_LIBS})

	target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${TINYGLTF_INCLUDE_DIR}")

	if (BUILD_CLIENT)
		# bgfx
		rge_abs_path(BGFX_INCLUDE_DIRS_ABS "${BGFX_INCLUDE_DIRS}")

		target_include_directories(${GAME_NAME} PUBLIC ${BGFX_INCLUDE_DIRS_ABS})
		target_link_libraries(${GAME_NAME} ${BGFX_LIBS})

		if (USE_GLFW)
			# GLFW
			target_include_directories(${GAME_NAME} PUBLIC "${RGE_HOME}/${GLFW_INCLUDE_DIRS}")
			target_link_libraries(${GAME_NAME} ${GLFW_LIBS})
		endif()
	endif()
endfunction()
