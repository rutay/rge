#pragma once

#include "resources/resource_manager.hpp"

namespace rge::resources {

enum class Shader {
None = 0 * 0x01000000,
GLSL_VS_BasicInst,
GLSL_FS_IterLights,
};

enum class Material {
None = 1 * 0x01000000,
Basic,
Phong,
Standard,
};

enum class MaterialShader {
None = 2 * 0x01000000,
GLSL_FS_IterLights_Basic,
GLSL_FS_IterLights_Phong,
GLSL_FS_IterLights_Standard,
};

enum class Model {
None = 3 * 0x01000000,
GumballDarwin,
McLaren,
};


inline static void init() {
rge::ResourceManager::register_resource_desc(Shader::GLSL_VS_BasicInst, ".rge/assets/cli/shaders/glsl/basic_inst.vert");
rge::ResourceManager::register_resource_desc(Shader::GLSL_FS_IterLights, ".rge/assets/cli/shaders/glsl/iter_lights.frag");
rge::ResourceManager::register_resource_desc(Material::Basic, ".rge/assets/cli/materials/basic.mat");
rge::ResourceManager::register_resource_desc(Material::Phong, ".rge/assets/cli/materials/phong.mat");
rge::ResourceManager::register_resource_desc(Material::Standard, ".rge/assets/cli/materials/standard.mat");
rge::ResourceManager::register_resource_desc(MaterialShader::GLSL_FS_IterLights_Basic, ".rge/assets/cli/material_shaders/glsl/iter_lights_basic.frag");
rge::ResourceManager::register_resource_binary_assoc(Shader::GLSL_FS_IterLights, Material::Basic, MaterialShader::GLSL_FS_IterLights_Basic);
rge::ResourceManager::register_resource_desc(MaterialShader::GLSL_FS_IterLights_Phong, ".rge/assets/cli/material_shaders/glsl/iter_lights_phong.frag");
rge::ResourceManager::register_resource_binary_assoc(Shader::GLSL_FS_IterLights, Material::Phong, MaterialShader::GLSL_FS_IterLights_Phong);
rge::ResourceManager::register_resource_desc(MaterialShader::GLSL_FS_IterLights_Standard, ".rge/assets/cli/material_shaders/glsl/iter_lights_standard.frag");
rge::ResourceManager::register_resource_binary_assoc(Shader::GLSL_FS_IterLights, Material::Standard, MaterialShader::GLSL_FS_IterLights_Standard);
rge::ResourceManager::register_resource_desc(Model::GumballDarwin, "assets/models/gumball_darwin.glb");
rge::ResourceManager::register_resource_desc(Model::McLaren, "assets/models/McLaren.glb");
}
}