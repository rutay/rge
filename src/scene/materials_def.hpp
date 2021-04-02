
#pragma once

#include "material.hpp"
#include "resources_def.hpp"

namespace rge::materials
{
template<Resource resource>
struct __BaseMaterial : public Material
{
	const Resource m_resource = resource;
	Resource get_resource() const override { return m_resource; };
};

struct BasicMaterial : public __BaseMaterial<resources::Basic>
{
	Vec4 m_color{ 0, 0, 0, 0 };
};

struct PhongMaterial : public __BaseMaterial<resources::Basic>
{

};

}
