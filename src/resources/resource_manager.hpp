
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

	namespace ResourceManager
	{
		namespace detail
		{
			inline std::unordered_map<Resource, ResourceDescriptor> s_resource_descriptors;
			inline std::unordered_map<uint32_t, Resource> s_resource_binary_assoc;
		}

		inline void register_resource_desc(Resource res, ResourceDescriptor res_desc)
		{
			detail::s_resource_descriptors.emplace(res, res_desc);
		}

		inline void register_resource_desc(Resource res, std::filesystem::path res_path)
		{
			register_resource_desc(res, {
				.m_path = res_path
			});
		}

		inline ResourceDescriptor const& get_resource_desc(Resource resource)
		{
			return detail::s_resource_descriptors[resource];
		}

		inline std::filesystem::path const& get_resource_path(Resource resource)
		{
			return get_resource_desc(resource).m_path;
		}

		inline void register_resource_binary_assoc(Resource res_a, Resource res_b, Resource res_c)
		{
			detail::s_resource_binary_assoc.emplace(((uint32_t)res_a) << 16 | res_b, res_c);
		}

		inline Resource get_resource_from_assoc(Resource res_a, Resource res_b)
		{
			return detail::s_resource_binary_assoc[((uint32_t)res_a) << 16 | res_b];
		}

		inline resources::MaterialShader get_material_shader(resources::Shader shader, resources::Material material)
		{
			return (resources::MaterialShader)(get_resource_from_assoc(shader, material));
		}

		inline resources::MaterialShader get_material_shader(resources::Shader shader, Material const* material)
		{
			return get_material_shader(shader, (resources::Material)material->get_resource());
		}
	};
}
