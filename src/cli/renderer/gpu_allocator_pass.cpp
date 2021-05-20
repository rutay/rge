#include "gpu_allocator.hpp"

using namespace rge;

void GpuAllocator::Pass::handle_geometry_update(Packet_GeometryUpdate const* pkt)
{
	DrawCall& draw_call = m_result->m_draw_calls.at(m_result->m_draw_call_idx_by_geometry.at(pkt->m_geometry));
	m_gpu_allocator->realloc_vertex_buffers(draw_call, pkt->m_geometry);
	m_gpu_allocator->realloc_indices_buffer(draw_call, pkt->m_geometry);
}

void GpuAllocator::Pass::handle_light_update(Packet_LightUpdate const* pkt)
{
	// todo
}

void GpuAllocator::Pass::handle_node_set_geometry(Packet_Node_SetGeometry const* pkt)
{
	if (pkt->m_old_geometry == pkt->m_new_geometry) {
		return;
	}

	if (pkt->m_old_geometry)
	{
		m_gpu_allocator->realloc_instances_buffer(
			m_result->m_draw_calls.at(m_result->m_draw_call_idx_by_geometry.at(pkt->m_old_geometry)),
			m_linearized_scene_graph->m_instances_by_geometry.at(pkt->m_old_geometry)
		);
	}

	if (pkt->m_new_geometry)
	{
		m_gpu_allocator->realloc_instances_buffer(
			m_result->m_draw_calls.at(m_result->m_draw_call_idx_by_geometry.at(pkt->m_new_geometry)),
			m_linearized_scene_graph->m_instances_by_geometry.at(pkt->m_new_geometry)
		);
	}
}

void GpuAllocator::Pass::handle_node_set_material(Packet_Node_SetMaterial const* pkt)
{
	// todo
}

void GpuAllocator::Pass::handle_node_set_light(Packet_NodeSetLight const* pkt)
{
	// todo
}

void process_node_update(GpuAllocator::Pass* pass, Node const* node)
{
	if (node->m_geometry)
	{
		GpuAllocator::DrawCall& draw_call = pass->m_result->m_draw_calls.at(pass->m_result->m_draw_call_idx_by_geometry.at(node->m_geometry));
		pass->m_gpu_allocator->update_instance_buffer_instance(draw_call, node);
	}
}

void GpuAllocator::Pass::handle_node_transform_update(Packet_NodeTransformUpdate const* pkt)
{
	process_node_update(this, pkt->m_node);
}

void GpuAllocator::Pass::handle_node_parent_update(Packet_NodeParentUpdate const* pkt)
{
	process_node_update(this, pkt->m_node); // Changing the parent, in this pass only means that the transform have changed.
}

void GpuAllocator::Pass::handle_all()
{
	// todo !!!!!
}
