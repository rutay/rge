
#pragma once

#include "util.hpp"

#include "material.hpp"
#include "resources_def.hpp"

namespace rge::materials
{
	namespace detail
	{
		template<resources::Material material_type>
		struct BaseMaterial : public Material
		{
			resources::Material get_resource() const override { return material_type; };
		};
	}

	struct BasicMaterial : public detail::BaseMaterial<resources::Material::Basic>
	{
		Vec4 m_color;
	};

	struct PhongMaterial : public detail::BaseMaterial<resources::Material::Phong>
	{
		Vec3 m_ambient;
		Vec3 m_diffuse;
		Vec3 m_specular;
		float m_shininess;
	};

	struct StandardMaterial : public detail::BaseMaterial<resources::Material::Standard>
	{
		float m_metallic;
		float m_roughness;
		Vec3 m_base_color;
	};
}
