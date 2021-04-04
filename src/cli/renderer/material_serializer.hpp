
#pragma once

#include "resources/resource.hpp"
#include "scene/material.hpp"

#include <unordered_map>

namespace rge
{
	class MaterialSerializer
	{
	public:
		virtual void serialize(Material const* material, uint8_t* data, size_t& size) const = 0;
	};

	namespace MaterialSerializerManager
	{
		namespace detail
		{
			inline std::unordered_map<resources::Material, MaterialSerializer*> s_serializer_by_material_type;
		}

		template<resources::Material material_type, typename TMaterialSerializer>
		void register_material_serializer()
		{
			static TMaterialSerializer material_serializer;
			detail::s_serializer_by_material_type.emplace(material_type, &material_serializer);
		}

		inline MaterialSerializer* get_material_serializer(resources::Material material_type)
		{
			return detail::s_serializer_by_material_type[material_type];
		}

		inline MaterialSerializer* get_material_serializer(Material const* material)
		{
			return get_material_serializer(material->get_resource());
		}

		inline void serialize(Material const* material, uint8_t* data, size_t& size)
		{
			get_material_serializer(material)->serialize(material, data, size);
		}
	}
}