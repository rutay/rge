
include_guard()

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

# --

macro (rge_set_g VAR_NAME VAR_VALUE)
	set(${VAR_NAME} "${VAR_VALUE}" CACHE INTERNAL "")

	list(APPEND RGE_G_VARS ${VAR_NAME})
	set(RGE_G_VARS "${RGE_G_VARS}" CACHE INTERNAL "")
endmacro()

function (rge_reset_glob_vars)
	list(LENGTH RGE_G_VARS RGE_G_VARS_LEN)
	rge_log(NOTICE "Clearing ${RGE_G_VARS_LEN} global variables")

	foreach (RGE_G_VAR ${RGE_G_VARS})
		unset(${RGE_G_VAR} CACHE)
		#message("clearing ${RGE_G_VAR}")
	endforeach()
	unset(RGE_G_VARS CACHE)
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
# Assets
# ------------------------------------------------------------------------------------------------

# `rge_compile_shader` <SHADER_FILENAME> <SHADER_TYPE> <SHADER_VARYINGDEF_FILENAME>
#
# * SHADER_FILENAME            - where to find the shader, relative to the assets/ folder.
# * SHADER_TYPE                - the type of the shader (v|f).
# * SHADER_VARYINGDEF_FILENAME - where to find the varyingdef,  relative to the assets/ folder.
# * SRC_DIR                    - the directory where to search assets/shaders/${SHADER_NAME}. By default: ${CMAKE_CURRENT_BINARY_DIR}.
# * DST_DIR                    - the directory where to put the assets/shaders/${SHADER_DST_NAME} (compiled).
function (rge_compile_shader GAME_NAME SHADER_NAME SHADER_TYPE VARYING_DEF_NAME SRC_DIR DST_DIR)
	if (NOT DEFINED SRC_DIR)
		set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
	endif()

	if (NOT DEFINED DST_DIR)
		set(DST_DIR $<TARGET_FILE_DIR:${GAME_NAME}>)
	endif()

	set(ASSETS_SRC_DIR "${SRC_DIR}/assets")
	set(ASSETS_DST_DIR "${DST_DIR}/assets")

	set(SHADERS_SRC_DIR "${ASSETS_SRC_DIR}/cli/shaders")
	set(SHADER_SRC "${SHADERS_SRC_DIR}/${SHADER_NAME}")
	set(VARYING_DEF_SRC "${SHADERS_SRC_DIR}/${VARYING_DEF_NAME}")

	set(SHADERS_DST_DIR "${ASSETS_DST_DIR}/cli/shaders")

	set(SHADER_DST_NAME "${SHADER_NAME}.bin")
	set(SHADER_DST "${SHADERS_DST_DIR}/${SHADER_DST_NAME}")

	add_custom_command(
		TARGET ${GAME_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DST_DIR}
		COMMAND $<TARGET_FILE:shaderc> -f ${SHADER_SRC} -i "${RGE_HOME}/${BGFX_SHADER_INCLUDE_DIR}" --type ${SHADER_TYPE} --varyingdef ${VARYING_DEF_SRC} -o ${SHADER_DST}
		MAIN_DEPENDENCY ${SHADER_SRC}
		DEPENDS shaderc ${VARYING_DEF_SRC}
		COMMENT "Compiling: ${SHADER_SRC} (${SHADER_TYPE})"
	)
endfunction()

function (rge_resolve_src_path RETURN_VAR SYMBOLIC_PATH)

	string(REGEX REPLACE "^\\.rge/" "${RGE_HOME}/" REAL_PATH ${SYMBOLIC_PATH})
	if (${REAL_PATH} STREQUAL ${SYMBOLIC_PATH}) # The symbolic path isn't starting with .rge/, then it has to be in the game's source directory.
		set(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${SYMBOLIC_PATH}")
	endif()

	set(${RETURN_VAR} ${REAL_PATH} PARENT_SCOPE)

endfunction()

function (rge_resolve_dst_path RETURN_VAR SYMBOLIC_PATH)
	set(${RETURN_VAR} "$<TARGET_FILE_DIR:${GAME_NAME}>/${SYMBOLIC_PATH}" PARENT_SCOPE) # The symbolic path is equivalent to the binary-relative path.
endfunction()

# ---

# RES_TYPES
# ${RESOURCE_TYPE}_${RESOURCE_VAR_NAME}_PATH
# ${RESOURCE_TYPE}_DEF
# ${RESOURCE_TYPE}_REG_DESC_FUNC
# RES_ASSOC_FUNC
# GAME_NAME

function (rge_def_resource TYPE VAR_NAME PATH)
	rge_log(NOTICE "Resource ${TYPE}: \"${VAR_NAME}\" ${PATH}")

	rge_resolve_src_path(SRC_PATH ${PATH})
	rge_resolve_dst_path(DST_PATH ${PATH})

	if (REQUESTED)
		add_custom_command(
			TARGET ${GAME_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${SRC_PATH} $<TARGET_FILE_DIR:${GAME_NAME}>/${DST_PATH}
			MAIN_DEPENDENCY ${SRC_PATH}
		)
	endif()

	list(APPEND ${TYPE}_DEF ${VAR_NAME})
	list(APPEND ${TYPE}_REG_DESC_FUNC "rge::ResourceManager::register_resource_desc(${TYPE}::${VAR_NAME}, \"${PATH}\")")

	rge_set_g(${TYPE}_${VAR_NAME}_PATH "${PATH}")
	rge_set_g(${TYPE}_DEF "${${TYPE}_DEF}")
	rge_set_g(${TYPE}_REG_DESC_FUNC "${${TYPE}_REG_DESC_FUNC}")
endfunction()

function (rge_def_shader VAR_NAME PATH)
	rge_def_resource(Shader ${VAR_NAME} ${PATH} ${REQUESTED})
endfunction()

function (rge_def_material VAR_NAME PATH)
	rge_def_resource(Material ${VAR_NAME} ${PATH} ${REQUESTED})
endfunction()

function (rge_def_material_shader VAR_NAME PATH)
	rge_def_resource(MaterialShader ${VAR_NAME} ${PATH} ${REQUESTED})
endfunction()

function (rge_def_model VAR_NAME PATH)
	rge_def_resource(Model ${VAR_NAME} ${PATH} ${REQUESTED})
endfunction()

# ---

function (rge_req_resource TYPE VAR_NAME)
	set(SYM_PATH ${TYPE}_${VAR_NAME}_PATH)

	rge_resolve_src_path(SRC_PATH ${SYM_PATH})
	rge_resolve_dst_path(DST_PATH ${SYM_PATH})

	if (NOT EXISTS ${SRC_PATH})
		return()
	endif()

	add_custom_command(
		TARGET ${GAME_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SRC_PATH} ${DST_PATH}
	)
endfunction()

function (rge_req_shader VAR_NAME)
	rge_req_resource(Shader ${VAR_NAME})
endfunction()

function (rge_req_model VAR_NAME)
	rge_req_resource(Model ${VAR_NAME})
endfunction()

# ---

function (rge_resource_assoc VAR1_TYPE VAR1_NAME VAR2_TYPE VAR2_NAME OUT_VAR_TYPE OUT_VAR_NAME)
	set(VAR1_NAME "${VAR1_TYPE}::${VAR1_NAME}")
	set(VAR2_NAME "${VAR2_TYPE}::${VAR2_NAME}")
	set(OUT_VAR_NAME "${OUT_VAR_TYPE}::${OUT_VAR_NAME}")

	list(APPEND RES_ASSOC_FUNC "rge::ResourceManager::register_resource_binary_assoc(${VAR1_NAME}, ${VAR2_NAME}, ${OUT_VAR_NAME})")

	rge_log(NOTICE "Resource binary association...")
	rge_log(STATUS "Var1: ${VAR1_NAME}")
	rge_log(STATUS "Var2: ${VAR2_NAME}")
	rge_log(STATUS "Output: ${OUT_VAR_NAME}")

	rge_set_g(RES_ASSOC_FUNC "${RES_ASSOC_FUNC}")
endfunction()

function (rge_compile_material_shader SHADER_NAME MATERIAL_NAME MATERIAL_SHADER_NAME)
	rge_resource_assoc(Shader ${SHADER_NAME} Material ${MATERIAL_NAME} MaterialShader ${MATERIAL_SHADER_NAME})

	set(SHADER_PATH "${Shader_${SHADER_NAME}_PATH}")
	set(MATERIAL_PATH "${Material_${MATERIAL_NAME}_PATH}")
	set(MATERIAL_SHADER_PATH "${MaterialShader_${MATERIAL_SHADER_NAME}_PATH}")

	rge_resolve_src_path(SHADER_PATH ${SHADER_PATH})
	rge_resolve_src_path(MATERIAL_PATH ${MATERIAL_PATH})
	rge_resolve_dst_path(MATERIAL_SHADER_PATH ${MATERIAL_SHADER_PATH})

	get_filename_component(MATERIAL_SHADER_DIR ${MATERIAL_SHADER_PATH} DIRECTORY)

	rge_log(NOTICE "Compiling material shader...")
	rge_log(STATUS "Shader: ${SHADER_PATH}")
	rge_log(STATUS "Material: ${MATERIAL_PATH}")
	rge_log(STATUS "Material shader: ${MATERIAL_SHADER_DIR}")

	add_custom_command(
			TARGET ${GAME_NAME}
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${MATERIAL_SHADER_DIR}
			COMMAND ${RGE_SHADERC} -i ${SHADER_PATH} -m ${MATERIAL_PATH} -o ${MATERIAL_SHADER_PATH}
			MAIN_DEPENDENCY ${SHADER_PATH}
	)
endfunction()

# ---

function (rge_submit_resources_def)
	set(DST_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.gen_cfg/resources_def.hpp")
	target_sources(${GAME_NAME} PRIVATE ${DST_PATH})

	string(APPEND GEN_CODE "#pragma once")
	string(APPEND GEN_CODE "\n")
	string(APPEND GEN_CODE "#include \"resources/resource_manager.hpp\"\n")
	string(APPEND GEN_CODE "\n")

	# Begin
	string(APPEND GEN_CODE "namespace rge::resources {\n\n")

	# Resources definition
	foreach (RES_TYPE ${RES_TYPES})
		string(APPEND GEN_CODE "enum ${RES_TYPE} {\n")

		list(JOIN ${RES_TYPE}_DEF ",\n" RES_DEF)
		string(APPEND GEN_CODE "${RES_DEF}\n")
		string(APPEND GEN_CODE "}\;\n\n")
	endforeach()

	# Init
	string(APPEND GEN_CODE "inline static void init() {\n")

	list(LENGTH RES_TYPES RES_TYPES_LEN)

	# Resources descriptor registration
	foreach (RES_TYPE ${RES_TYPES})

		if (${RES_TYPE}_REG_DESC_FUNC)
			list(JOIN ${RES_TYPE}_REG_DESC_FUNC "\;\n" ${RES_TYPE}_REG_DESC_FUNC_STR)
			string(APPEND GEN_CODE "${${RES_TYPE}_REG_DESC_FUNC_STR}\;\n")
		endif()
	endforeach()

	string(APPEND GEN_CODE "\n")

	# Resources assoc func
	if (RES_ASSOC_FUNC)
		list(JOIN RES_ASSOC_FUNC "\;\n" RES_ASSOC_FUNC_STR)
		string(APPEND GEN_CODE "${RES_ASSOC_FUNC_STR}\;\n")
	endif()

	string(APPEND GEN_CODE "}\n")

	# End
	string(APPEND GEN_CODE "}\n")

	file(WRITE ${DST_PATH} ${GEN_CODE})
endfunction()

# ---

function (rge_end_game)
	rge_submit_resources_def()
endfunction()

function (rge_submit_material_shader GAME_NAME MATERIAL_SHADERS_DEF)

	set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/material_shaders_def.cpp")

	configure_file("${RGE_HOME}/glue/material_shaders_def.cpp" ${OUTPUT})
	target_sources(${GAME_NAME} PRIVATE "${OUTPUT}")

endfunction()

# ------------------------------------------------------------------------------------------------
# Game
# ------------------------------------------------------------------------------------------------

# `rge_define_game` <GAME_NAME>
#
# * GAME_NAME    - the name of the game executable created.
function (rge_begin_game GAME_NAME SRCS)

	rge_reset_glob_vars()

	rge_set_g(GAME_NAME ${GAME_NAME})

	rge_log(NOTICE "Game definition: ${GAME_NAME}")

	# Prepares a list containing the RGE's sources and the game's sources.
	# The engine sources must be taken according to the set RGE_HOME.
	rge_abs_path(FINAL_SRCS "${RGE_SRC}")
	list(APPEND FINAL_SRCS ${SRCS})

	add_executable(${GAME_NAME} ${FINAL_SRCS})

	# ----------------------------------------------------------------
	# Assets
	# ----------------------------------------------------------------

	rge_set_g(RES_TYPES "Shader;Material;MaterialShader;Model;Texture")

	rge_def_shader("Glsl_VS_BasicInst" ".rge/assets/cli/shaders/glsl/basic_inst.vert")
	rge_req_shader("Glsl_VS_BasicInst")

	rge_def_shader("Glsl_FS_IterLights" ".rge/assets/cli/shaders/glsl/iter_lights.frag")

	rge_def_material("Basic" ".rge/assets/cli/materials/basic.mat")
	rge_def_material("Phong" ".rge/assets/cli/materials/phong.mat")
	rge_def_material("Standard" ".rge/assets/cli/materials/standard.mat")

	rge_def_material_shader("Glsl_FS_IterLights_Basic" ".rge/assets/cli/material_shaders/glsl/iter_lights_basic.frag")
	rge_compile_material_shader("Glsl_FS_IterLights" "Basic" "Glsl_FS_IterLights_Basic")

	rge_def_material_shader("Glsl_FS_IterLights_Phong" ".rge/assets/cli/material_shaders/glsl/iter_lights_phong.frag")
	rge_compile_material_shader("Glsl_FS_IterLights" "Phong" "Glsl_FS_IterLights_Phong")

	rge_def_material_shader("Glsl_FS_IterLights_Standard" ".rge/assets/cli/material_shaders/glsl/iter_lights_standard.frag")
	rge_compile_material_shader("Glsl_FS_IterLights" "Standard" "Glsl_FS_IterLights_Standard")

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

