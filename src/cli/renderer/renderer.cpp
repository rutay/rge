#include "renderer.hpp"

#include <iostream>

#include "material_program.hpp"

using namespace rge::renderer;

bgfx::Attrib::Enum bgfx_parse_attrib_type(AttribType attrib_type)
{
	switch (attrib_type) {
	case AttribType::POSITION:   return bgfx::Attrib::Enum::Position;
	case AttribType::NORMAL:     return bgfx::Attrib::Enum::Normal;
	case AttribType::TEXCOORD_0: return bgfx::Attrib::Enum::TexCoord0;
	case AttribType::TEXCOORD_1: return bgfx::Attrib::Enum::TexCoord1;
	case AttribType::COLOR_0:    return bgfx::Attrib::Enum::Color0;
	case AttribType::WEIGHTS_0:  return bgfx::Attrib::Enum::Weight;
	default:                     return bgfx::Attrib::Enum::Count; // todo throw an error
	}
}

bgfx::AttribType::Enum bgfx_parse_component_type(ComponentType comp_type)
{
	switch (comp_type) {
	case ComponentType::FLOAT:         return bgfx::AttribType::Enum::Float;
	case ComponentType::SHORT:         return bgfx::AttribType::Enum::Int16;
	case ComponentType::UNSIGNED_BYTE: return bgfx::AttribType::Enum::Uint8;
	default:                           return bgfx::AttribType::Enum::Count; // todo throw an error
	}
}

void bake_instance_data(bgfxRenderer::InstanceData& instance_data, std::vector<Node const*> const& nodes)
{
	bgfx::Memory const* memory = bgfx::alloc(nodes.size() * 16 * sizeof(float));
	uint8_t* data = memory->data;
	for (Node const* node : nodes)
	{
		bx::memCopy(data, node->m_world_transform, 16 * sizeof(float));
		data += 16 * sizeof(float);
	}
	bgfx::VertexLayout vertex_layout;
	vertex_layout
		.begin()
		.skip(16 * sizeof(float))
		.end();

	instance_data.m_buffer = bgfx::createVertexBuffer(memory, vertex_layout),
	instance_data.m_count = nodes.size();
}

void bake_geometry(bgfxRenderer::BakedGeometry& baked_geometry, Geometry const* geometry)
{
	printf("Baking geometry %p\n", geometry);

	for (int attrib_type = 0; attrib_type < (int) AttribType::Count; attrib_type++)
	{
		Accessor const* accessor = geometry->m_attributes[attrib_type];
		if (accessor != nullptr)
		{
			printf("Creating VBO for attrib type: %d\n", attrib_type);

			BufferView const* buffer_view = accessor->m_buffer_view;

			bgfx::VertexLayout vertex_layout;
			vertex_layout
				.begin()
				.add(bgfx_parse_attrib_type(static_cast<AttribType>(attrib_type)), accessor->m_num_components, bgfx_parse_component_type(accessor->m_component_type))
				.skip(buffer_view->m_byte_stride)
				.end();

			size_t byte_offset = buffer_view->m_byte_offset + accessor->m_byte_offset;
			bgfx::Memory const* bgfx_memory = bgfx::makeRef(
				static_cast<unsigned char*>(buffer_view->m_buffer->m_data) + byte_offset,
				buffer_view->m_byte_length - byte_offset
			);
			bgfx::VertexBufferHandle vbo = bgfx::createVertexBuffer(bgfx_memory, vertex_layout);
			baked_geometry.m_vertex_buffers.push_back(vbo);
		}
	}

	if (geometry->m_indices != nullptr)
	{
		printf("Creating IBO\n");

		Accessor const* accessor = geometry->m_indices;
		BufferView const* buffer_view = accessor->m_buffer_view;

		uint16_t indices_type;
		if (accessor->m_component_type == ComponentType::UNSIGNED_SHORT)    indices_type = BGFX_BUFFER_NONE;
		else if (accessor->m_component_type == ComponentType::UNSIGNED_INT) indices_type = BGFX_BUFFER_INDEX32;
		else {
			printf("Couldn't match the requested IBO component type: %d.", accessor->m_component_type);
			return; // todo throw error
		}

		size_t byte_offset = buffer_view->m_byte_offset + accessor->m_byte_offset;
		bgfx::Memory const* bgfx_memory = bgfx::makeRef(
			static_cast<unsigned char*>(accessor->m_buffer_view->m_buffer->m_data) + byte_offset,
			buffer_view->m_byte_length - byte_offset
		);
		bgfx::IndexBufferHandle ibo = bgfx::createIndexBuffer(bgfx_memory, indices_type);
		baked_geometry.m_index_buffer = ibo;
	}
}

void bgfxRenderer::render(bgfx::ViewId view_id, Node const* root_node, Camera const& camera)
{
	if (!m_mesh_instances_by_root_node.contains(root_node))
	{
		printf("Creating IDB for root node %p\n", root_node);

		std::unordered_map<Mesh const*, std::vector<Node const*>> nodes_by_mesh;
		root_node->traverse_const([&](Node const* node) {
			for (Mesh const* mesh : node->m_meshes)
			{
				nodes_by_mesh.insert({ mesh, std::vector<Node const*>() });
				nodes_by_mesh[mesh].push_back(node);
			}
		});

		m_mesh_instances_by_root_node.insert({ root_node, std::vector<std::tuple<Mesh const*, InstanceData>>(nodes_by_mesh.size()) });
		auto& mesh_instances = m_mesh_instances_by_root_node[root_node];

		for (auto &[mesh, nodes] : nodes_by_mesh)
		{
			InstanceData instance_data;
			bake_instance_data(instance_data, nodes);
			mesh_instances.emplace_back(mesh, instance_data);
		}
	}

	for (auto &[mesh, instance_data] : m_mesh_instances_by_root_node.at(root_node))
	{
		if (!mesh) {
			continue;
		}

		Geometry const* geometry = mesh->m_geometry;
		Material const* material = mesh->m_material;

		if (!geometry || !material) {
			continue;
		}

		if (!m_baked_geometries.contains(geometry))
		{
			BakedGeometry baked_geometry;
			bake_geometry(baked_geometry, geometry);
			m_baked_geometries.insert({ geometry, baked_geometry });
		}
		BakedGeometry& baked_geometry = m_baked_geometries[geometry];

		for (int stream = 0; stream < baked_geometry.m_vertex_buffers.size(); stream++) {
			bgfx::setVertexBuffer(stream, baked_geometry.m_vertex_buffers[stream]);
		}
		bgfx::setIndexBuffer(baked_geometry.m_index_buffer);

		bgfx::setInstanceDataBuffer(instance_data.m_buffer, 0, instance_data.m_count);

		float camera_view[16], camera_proj[16];
		camera.view_matrix(camera_view);
		camera.projection_matrix(camera_proj);
		bgfx::setViewTransform(view_id, camera_view, camera_proj);

		bgfx::setState(BGFX_STATE_DEFAULT);

		MaterialProgram* program = MaterialProgramRegistry::get(material);
		program->set_material(material);
		program->run(view_id);
	}
}
