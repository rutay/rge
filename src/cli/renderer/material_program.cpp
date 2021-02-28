
#include "material_program.hpp"

#include "rge.hpp"
#include "resource_provider.hpp"

using namespace rge::renderer;

void load_shader(bgfx::ShaderHandle& shader, char const* src_path)
{
	std::vector<uint8_t> buffer;
	int size = rge::resource_provider::read(src_path, buffer);
	if (size < 0) {
		// todo error
		return;
	}
	bgfx::Memory const* memory = bgfx::copy(buffer.data(), buffer.size());
	shader = bgfx::createShader(memory);
	bgfx::setName(shader, src_path);
}

// ------------------------------------------------------------------------------------------------
// MaterialProgram
// ------------------------------------------------------------------------------------------------

void MaterialProgram::run(bgfx::ViewId view_id) const
{
	bgfx::submit(view_id, m_program);
}

// ------------------------------------------------------------------------------------------------
// MaterialProgramRegistry
// ------------------------------------------------------------------------------------------------

std::vector<MaterialProgram*> MaterialProgramRegistry::m_material_programs = {};

void MaterialProgramRegistry::__register(int8_t material_id, MaterialProgram* material_program)
{
	if (material_id < 0) {
		// todo throw error material should be registered before usage
		return;
	}

	if (m_material_programs.size() <= material_id) {
		m_material_programs.resize(material_id + 1);
	}
	m_material_programs[material_id] = material_program;
	material_program->init();
}

void MaterialProgramRegistry::init()
{
	static BasicMaterialProgram basic_material_program;

	RGE_MATERIAL_ASSOCIATE_PROGRAM(BasicMaterial, &basic_material_program);
}

MaterialProgram* MaterialProgramRegistry::get(Material const* material)
{
	if (material->get_type_id() >= m_material_programs.size()) {
		// todo throw error
		return nullptr;
	}
	return m_material_programs[material->get_type_id()];
}

// ------------------------------------------------------------------------------------------------
// BasicMaterialProgram
// ------------------------------------------------------------------------------------------------

void BasicMaterialProgram::init()
{
	bgfx::ShaderHandle vertex_shader, fragment_shader;
	load_shader(vertex_shader, RGE_FILEPATH("assets/cli/shaders/basic.vs.bin"));
	load_shader(fragment_shader, RGE_FILEPATH("assets/cli/shaders/basic.fs.bin"));
	m_program = bgfx::createProgram(vertex_shader, fragment_shader, true);

	m_color_uniform = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
}

void BasicMaterialProgram::set_material(Material const* material) const
{
	auto basic_material = dynamic_cast<BasicMaterial const*>(material);

	bgfx::setUniform(m_color_uniform, basic_material->m_color);
}


