
#pragma once

#include <vector>
#include <array>
#include <unordered_map>

#include "resources_def.hpp"

#include "renderer.hpp"
#include <glad/glad.h>

namespace rge
{
class RendererGL : public Renderer
{
protected:
	SDL_GLContext m_gl_context;

public:
	struct BakedAttribute
	{
		GLuint m_vertex_array;
		GLuint m_vertex_buffer;
	};

	struct BakedGeometry
	{
		BakedAttribute m_baked_attributes[AttribType::Count];

		GLenum m_indices_type;
		GLuint m_indices_buffer;

		size_t m_instances_count;
	};

	std::unordered_map<Geometry const*, BakedGeometry> m_baked_geometries;
	std::unordered_map<Node const*, std::vector<std::tuple<Mesh const*, GLuint>>> m_baked_instances_by_root_node;
	std::unordered_map<Node const*, GLuint> m_baked_lights_by_root_node;

	std::unordered_map<resources::Material, GLuint> m_program_by_material_type;
	std::unordered_map<Material const*, GLuint> m_ubo_by_material;

	BakedGeometry& get_or_bake_geometry(Geometry const* geometry);
	GLuint get_or_create_program(Material const* material);
	GLuint get_or_bake_material_ubo(Material const* material);

	void render(Node const* root_node, Camera const& camera) override;
};
}
