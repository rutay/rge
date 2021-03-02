#pragma once

#include <vector>
#include "util.hpp"

namespace rge::scene
{
#define RGE_POLYMORPHIC_PARENT_DEFINITION \
	inline static uint8_t s_next_id = 0; \
	virtual uint8_t get_type_id() const = 0

#define RGE_POLYMORPHIC_CHILD_DEFINITION(Parent) \
	inline static int8_t s_id = Material::s_next_id++; \
	uint8_t get_type_id() const { return s_id; }

#define RGE_MATERIAL_DEFINITION \
	RGE_POLYMORPHIC_CHILD_DEFINITION(Material)

struct Material
{
	RGE_POLYMORPHIC_PARENT_DEFINITION;
};

struct BasicMaterial : public Material
{
	RGE_MATERIAL_DEFINITION;

	Vec4 m_color{ 0, 0, 0, 0 };
};

/*
struct PhongMaterial : public Material
{
	RGE_MATERIAL_DEFINITION;

	Vec3 m_ambient;
	Vec3 m_diffuse;
	Vec3 m_specular;
	float m_shininess = 0;
};
 */

struct PbrMaterial : public Material
{
	RGE_MATERIAL_DEFINITION;

	Vec3  m_base_color;
	float m_metallic;
	float m_roughness;
	float m_ao;
};
}
