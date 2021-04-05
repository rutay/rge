
#pragma once

#include "resource.hpp"

#include "scene/material.hpp"

#include <unordered_map>
#include <filesystem>

namespace rge
{
	struct ResourceDescriptor
	{
		std::filesystem::path m_path;
	};

	namespace ResourceManager
	{
		namespace detail
		{
			inline std::unordered_map<Resource, ResourceDescriptor> s_resource_descriptors;
			inline std::unordered_map<uint64_t, Resource> s_resource_binary_assoc;
		}

		template<typename R>
		inline void register_resource_desc(R res, ResourceDescriptor res_desc)
		{
			detail::s_resource_descriptors.emplace(static_cast<Resource>(res), res_desc);
		}

		template<typename R>
		inline void register_resource_desc(R res, std::filesystem::path res_path)
		{
			register_resource_desc(res, {
				.m_path = res_path
			});
		}

		template<typename R>
		inline ResourceDescriptor const& get_resource_desc(R resource)
		{
			return detail::s_resource_descriptors[static_cast<Resource>(resource)];
		}

		template<typename R>
		inline std::filesystem::path const& get_resource_path(R resource)
		{
			return get_resource_desc(resource).m_path;
		}

		template<is_resource_type A, is_resource_type B, is_resource_type R>
		inline void register_resource_binary_assoc(A res_a, B res_b, R res_c)
		{
			detail::s_resource_binary_assoc.emplace(((uint64_t) res_a) << 32 | (uint64_t) res_b, (Resource) res_c);
		}

		template<is_resource_type A, is_resource_type B, is_resource_type R>
		inline R get_resource_from_assoc(A res_a, B res_b)
		{
			Resource res_a_id = static_cast<Resource>(res_a);
			Resource res_b_id = static_cast<Resource>(res_b);
			return static_cast<R>(detail::s_resource_binary_assoc[((uint32_t) res_a_id) << 16 | res_b_id]);
		}

		inline resources::MaterialShader get_material_shader(resources::Shader shader, resources::Material material)
		{
			return get_resource_from_assoc<resources::Shader, resources::Material, resources::MaterialShader>(shader, material);
		}

		inline resources::MaterialShader get_material_shader(resources::Shader shader, Material const* material)
		{
			return get_material_shader(shader, (resources::Material)material->get_resource());
		}
	};
}
