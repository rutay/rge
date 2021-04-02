#pragma once
#include "resources/resource_manager.hpp"

namespace rge::resources {

enum Shader {
Glsl_VS_BasicInst,
Glsl_FS_IterLights
};

enum Material {
Basic
};

enum MaterialShader {
Glsl_FS_IterLights_Basic
};

enum Model {
McLaren,
GumballAndDarwin
};

enum Texture {

};

inline static void init() {
rge::ResourceManager::register_resource_desc(Shader::Glsl_VS_BasicInst, ".rge/assets/cli/shaders/glsl/basic_inst.vert");
rge::ResourceManager::register_resource_desc(Shader::Glsl_FS_IterLights, ".rge/assets/cli/shaders/glsl/iter_lights.frag");
rge::ResourceManager::register_resource_desc(Material::Basic, ".rge/assets/cli/materials/basic.json");
rge::ResourceManager::register_resource_desc(MaterialShader::Glsl_FS_IterLights_Basic, ".rge/assets/cli/material_shaders/glsl/iter_lights_basic.frag");
rge::ResourceManager::register_resource_desc(Model::McLaren, "assets/models/McLaren.glb");
rge::ResourceManager::register_resource_desc(Model::GumballAndDarwin, "assets/models/gumball_darwin.glb");

rge::ResourceManager::register_resource_binary_assoc(Shader::Glsl_FS_IterLights, Material::Basic, MaterialShader::Glsl_FS_IterLights_Basic);
}
}
