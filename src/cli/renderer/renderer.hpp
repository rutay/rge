#pragma once

#include <set>

#include <bgfx/bgfx.h>

#include "scene/scene.hpp"
#include "camera.hpp"

namespace rge::renderer
{
using namespace rge::scene;

struct bgfxRenderer
{
	struct BakedGeometry
	{
		std::vector<bgfx::VertexBufferHandle> m_vertex_buffers;
		bgfx::IndexBufferHandle m_index_buffer = BGFX_INVALID_HANDLE;
	};
	std::unordered_map<Geometry const*, BakedGeometry> m_baked_geometries;

	struct InstanceData
	{
		bgfx::VertexBufferHandle m_buffer;
		size_t m_count;
	};
	std::unordered_map<Node const*, std::vector<std::tuple<Mesh const*, InstanceData>>> m_mesh_instances_by_root_node;

	void render(bgfx::ViewId view_id, Node const* root_node, Camera const& camera);
};
}
