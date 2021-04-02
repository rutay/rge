
#include "material_handler.hpp"

using namespace rge;

void MaterialRegistry::init()
{
	static SpecializedMaterialSerializer<BasicMaterial> basic_material_serializer;
	MaterialRegistry::register_material_descriptor<BasicMaterial>("basic.mat", &basic_material_serializer);

	static SpecializedMaterialSerializer<PhongMaterial> phong_material_serializer;
	MaterialRegistry::register_material_descriptor<PhongMaterial>("phong.mat", &phong_material_serializer);

	static SpecializedMaterialSerializer<PBRMaterial> pbr_material_serializer;
	MaterialRegistry::register_material_descriptor<PBRMaterial>("pbr.mat", &pbr_material_serializer);
}

template<typename TMaterial> requires is_material<TMaterial>
void MaterialRegistry::register_material_descriptor(std::filesystem::path filename, SpecializedMaterialSerializer<TMaterial> const* serializer)
{
	s_material_descriptors[TMaterial::s_id] = {
		.m_filename = filename,
		.m_serializer = serializer
	};
}

MaterialDescriptor const& MaterialRegistry::get_material_descriptor(Material const* material)
{
	return s_material_descriptors[material->get_type_id()];
}

void MaterialRegistry::serialize_material(Material const* material, uint8_t* data, size_t size)
{
	get_material_descriptor(material).m_serializer->serialize(material, data, size);
}

void ShaderRegistry::register_material_shader(std::filesystem::path shader_path, std::filesystem::path material_path, std::filesystem::path output)
{
	s_material_based_shaders.insert({{shader_path, material_path}, output});
}

std::filesystem::path ShaderRegistry::get_material_based_shader(std::filesystem::path shader_path, std::filesystem::path material_path)
{
	return s_material_based_shaders[{shader_path, material_path}];
}

std::filesystem::path ShaderRegistry::get_material_based_shader(std::filesystem::path shader_path, Material const* material)
{
	return get_material_based_shader(shader_path, MaterialRegistry::get_material_descriptor(material).m_filename);
}
