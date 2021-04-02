
#pragma once

#include "resource.hpp"

#include "scene/material.hpp"

#include <unordered_map>
#include <filesystem>

namespace rge
{

namespace resources
{
enum Shader;
enum MaterialShader;
enum Material;
}

struct ResourceDescriptor
{
	std::filesystem::path m_path;
};

class ResourceManager
{
private:
	static std::unordered_map<Resource, ResourceDescriptor> s_resource_descriptors;
	static std::unordered_map<uint32_t, Resource> s_resource_binary_assoc;

	ResourceManager() = default;

public:
	static void register_resource_desc(Resource res, ResourceDescriptor res_desc)
	{
		s_resource_descriptors.emplace(res, res_desc);
	}

	static void register_resource_desc(Resource res, std::filesystem::path res_path)
	{
		register_resource_desc(res, {
			.m_path = res_path
		});
	}

	static ResourceDescriptor const& get_resource_desc(Resource resource)
	{
		return s_resource_descriptors[resource];
	}

	static std::filesystem::path const& get_resource_path(Resource resource)
	{
		return get_resource_desc(resource).m_path;
	}

	static void register_resource_binary_assoc(Resource res_a, Resource res_b, Resource res_c)
	{
		s_resource_binary_assoc.emplace(((uint32_t) res_a) << 16 | res_b, res_c);
	}

	static Resource get_resource_from_assoc(Resource res_a, Resource res_b)
	{
		return s_resource_binary_assoc.at(((uint32_t) res_a) << 16 | res_b);
	}

	static resources::MaterialShader get_material_shader(resources::Shader shader, resources::Material material)
	{
		return (resources::MaterialShader) (get_resource_from_assoc(shader, material));
	}

	static resources::MaterialShader get_material_shader(resources::Shader shader, Material const* material)
	{
		return get_material_shader(shader, (resources::Material) material->get_resource());
	}
};
}
