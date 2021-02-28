#pragma once

#include <vector>

#include "scene/material.hpp"

#include <bgfx/bgfx.h>

namespace rge::renderer
{
using namespace rge::scene;

class MaterialProgram
{
protected:
	bgfx::ProgramHandle m_program = BGFX_INVALID_HANDLE;

public:
	virtual void init() = 0;

	virtual void set_material(Material const* material) const = 0;
	void run(bgfx::ViewId view_id, uint32_t depth = 0, uint8_t flags = BGFX_DISCARD_ALL) const;
};

//

class MaterialProgramRegistry
{
private:
	static std::vector<MaterialProgram*> m_material_programs;

public:
	static void __register(int8_t material_id, MaterialProgram* material_program);
	static void init();

	static MaterialProgram* get(Material const* material);
};

#define RGE_MATERIAL_ASSOCIATE_PROGRAM(Material, material_program) \
	MaterialProgramRegistry::__register(Material::s_id, material_program);

//

class BasicMaterialProgram : public MaterialProgram
{
protected:
	bgfx::UniformHandle m_color_uniform;

public:
	void init() override;
	void set_material(Material const* material) const override;
};
}
