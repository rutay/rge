
#pragma once

#include "material_serializer.hpp"
#include "scene/materials_def.hpp"

namespace rge::material_serializers
{
	namespace detail
	{
		template<is_material TMaterial>
		class BaseMaterialSerializer : public MaterialSerializer
		{
		public:
			virtual void serialize_spec(TMaterial const* material, uint8_t* data, size_t& size) const
			{
				reinterpret_cast<TMaterial*>(data)[0] = *material;
				size = sizeof(TMaterial);
			}

			void serialize(Material const* material, uint8_t* data, size_t& size) const override
			{
				serialize_spec(static_cast<TMaterial const*>(material), data, size);
			}
		};
	}

	using BasicMaterialSerializer = detail::BaseMaterialSerializer<materials::BasicMaterial>;
	using PhongMaterialSerializer = detail::BaseMaterialSerializer<materials::PhongMaterial>;
	using StandardMaterialSerializer = detail::BaseMaterialSerializer<materials::StandardMaterial>;

	void init()
	{
		MaterialSerializerManager::register_material_serializer<resources::Material::Basic, material_serializers::BasicMaterialSerializer>();
		MaterialSerializerManager::register_material_serializer<resources::Material::Phong, material_serializers::PhongMaterialSerializer>();
		MaterialSerializerManager::register_material_serializer<resources::Material::Standard, material_serializers::StandardMaterialSerializer>();
	}
}
