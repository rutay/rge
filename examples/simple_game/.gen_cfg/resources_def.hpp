#pragma once
#include "resources/resource_manager.hpp"

namespace rge::resources {

enum class Shader {
None = 0,
Glsl_VS_BasicInst,
Glsl_FS_IterLights
};

enum class Material {
None = 16777216,
Basic,
Phong,
Standard
};

enum class MaterialShader {
None = 33554432,
Glsl_FS_IterLights_Basic,
Glsl_FS_IterLights_Phong,
Glsl_FS_IterLights_Standard
};

enum class Model {
None = 50331648,
McLaren,
GumballAndDarwin
};

enum class Texture {
None = 67108864,

};

inline static void init() {
rge::ResourceManager::register_resource_desc(Shader::Glsl_VS_BasicInst, ".rge/assets/cli/shaders/glsl/basic_inst.vert");
rge::ResourceManager::register_resource_desc(Shader::Glsl_FS_IterLights, ".rge/assets/cli/shaders/glsl/iter_lights.frag");
rge::ResourceManager::register_resource_desc(Material::Basic, ".rge/assets/cli/materials/basic.mat");
rge::ResourceManager::register_resource_desc(Material::Phong, ".rge/assets/cli/materials/phong.mat");
rge::ResourceManager::register_resource_desc(Material::Standard, ".rge/assets/cli/materials/standard.mat");
rge::ResourceManager::register_resource_desc(MaterialShader::Glsl_FS_IterLights_Basic, ".rge/assets/cli/material_shaders/glsl/iter_lights_basic.frag");
rge::ResourceManager::register_resource_desc(MaterialShader::Glsl_FS_IterLights_Phong, ".rge/assets/cli/material_shaders/glsl/iter_lights_phong.frag");
rge::ResourceManager::register_resource_desc(MaterialShader::Glsl_FS_IterLights_Standard, ".rge/assets/cli/material_shaders/glsl/iter_lights_standard.frag");
rge::ResourceManager::register_resource_desc(Model::McLaren, "assets/models/McLaren.glb");
rge::ResourceManager::register_resource_desc(Model::GumballAndDarwin, "assets/models/gumball_darwin.glb");

rge::ResourceManager::register_resource_binary_assoc(Shader::Glsl_FS_IterLights, Material::Basic, MaterialShader::Glsl_FS_IterLights_Basic);
rge::ResourceManager::register_resource_binary_assoc(Shader::Glsl_FS_IterLights, Material::Phong, MaterialShader::Glsl_FS_IterLights_Phong);
rge::ResourceManager::register_resource_binary_assoc(Shader::Glsl_FS_IterLights, Material::Standard, MaterialShader::Glsl_FS_IterLights_Standard);
}
}
