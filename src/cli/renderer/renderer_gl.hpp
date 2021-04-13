
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
public:
	struct BakedAttribute
	{
		GLuint m_vao;
		GLuint m_vbo;
	};

	struct BakedGeometry
	{
	    std::vector<BakedAttribute> m_baked_attribs;

        GLuint m_ebo;
		GLenum m_ebo_elem_type;
		size_t m_ebo_elem_count;

		size_t m_ibo_inst_count;
	};

	struct LightsUBO
	{
		GLuint m_idx;
		size_t m_lights_count;
	};

	std::unordered_map<Geometry const*, BakedGeometry> m_baked_geometries;
	std::unordered_map<Node const*, std::vector<std::tuple<Mesh const*, GLuint>>> m_baked_instances_by_root_node;
	std::unordered_map<Node const*, LightsUBO> m_lights_ubo_by_scene_graph;

	std::unordered_map<resources::Material, GLuint> m_program_by_material_type;
	std::unordered_map<Material const*, GLuint> m_ubo_by_material;

	void bake_scene_graph(Node const* scene_graph);
	BakedGeometry& get_or_bake_geometry(Geometry const* geometry);
	GLuint get_or_create_mat_program(Material const* material);
	GLuint get_or_bake_mat_ubo(Material const* material);

	void render(Node const* root_node, Camera const& camera) override;
};
}
