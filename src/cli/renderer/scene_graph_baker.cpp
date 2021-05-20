
#include "scene_graph_baker.hpp"

#include "vk_utils.hpp"

using namespace rge;
using namespace rge::scene_graph_baker;

using namespace std::placeholders;

// ------------------------------------------------------------------------------------------------ scene_graph_baker

void scene_graph_baker::init()
{
	packet_handler_class_registry::register_packet_handler_class(g_packet_handler_class, "scene_graph_baker_packet_handler");
}

// ------------------------------------------------------------------------------------------------ InstanceBuffer

bool InstanceBuffer::is_allocated()
{
	return m_buffer.is_allocated();
}

// ------------------------------------------------------------------------------------------------ DrawCall

/* Vertex buffers */
// One binding (or buffer) for every attribute (`separated`) or all attributes in one buffer (`interleaved`).
// TODO be able to choose between these two setups.

std::vector<VkVertexInputBindingDescription> DrawCall::get_vertex_buffers_binding_descriptions()
{
	std::vector<VkVertexInputBindingDescription> binding_descriptions;
	binding_descriptions.resize(AttribType::PerVertexAttribCount, {});

	for (size_t attrib_type = 0; attrib_type < AttribType::PerVertexAttribCount; attrib_type++)
	{
		Format::Enum attrib_format = AttribType::get_format(static_cast<AttribType::Enum>(attrib_type));

		VkVertexInputBindingDescription binding_description{};
		binding_description.binding = attrib_type;
		binding_description.stride = Format::get_size(attrib_format);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
		binding_descriptions[attrib_type] = binding_description;
	}

	return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> DrawCall::get_vertex_buffers_attributes_descriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributes_descriptions;
	attributes_descriptions.resize(AttribType::PerVertexAttribCount, {});

	for (size_t attrib_type = 0; attrib_type < AttribType::PerVertexAttribCount; attrib_type++)
	{
		Format::Enum attrib_format = AttribType::get_format(static_cast<AttribType::Enum>(attrib_type));

		VkVertexInputAttributeDescription attribute_description{};
		attribute_description.binding = attrib_type;
		attribute_description.location = attrib_type;
		attribute_description.format = to_vk_format(attrib_format);
		attribute_description.offset = 0;
		attributes_descriptions[attrib_type] = attribute_description;
	}
}

/* Instance buffer */
// Only one binding (or buffer) and its fields are interleaved.

VkVertexInputBindingDescription DrawCall::get_instance_buffer_binding_description()
{
	VkVertexInputBindingDescription binding_description{};
	binding_description.binding = AttribType::TRANSFORM_0;
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

	size_t stride = 0;
	for (size_t attrib_type = 0; attrib_type < AttribType::PerInstanceAttribCount; attrib_type++)
	{
		Format::Enum attrib_format = AttribType::get_format(static_cast<AttribType::Enum>(attrib_type));
		stride += Format::get_size(attrib_format);
	}
	binding_description.stride = stride;

	return binding_description;
}

std::vector<VkVertexInputAttributeDescription> DrawCall::get_instance_buffer_attributes_descriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributes_descriptions;
	attributes_descriptions.resize(AttribType::PerInstanceAttribCount, {});

	size_t offset = 0;

	VkVertexInputAttributeDescription attribute_description;
	attribute_description.binding = AttribType::TRANSFORM_0;

	for (size_t attrib_type = 0; attrib_type < AttribType::PerInstanceAttribCount; attrib_type++)
	{
		attribute_description.location = AttribType::TRANSFORM_0 + attrib_type;
		attribute_description.format = to_vk_format(AttribType::get_format(AttribType::TRANSFORM_0));
		attribute_description.offset = offset;
		attributes_descriptions[attrib_type] = attribute_description;
	}

	return attributes_descriptions;
}

/**/

std::vector<VkVertexInputBindingDescription> DrawCall::get_binding_descriptions()
{
	std::vector<VkVertexInputBindingDescription> binding_descriptions;

	auto vertex_buffers_binding_descriptions = get_vertex_buffers_binding_descriptions();
	auto instance_buffer_binding_description = get_instance_buffer_binding_description();

	binding_descriptions.insert(binding_descriptions.begin(), vertex_buffers_binding_descriptions.begin(), vertex_buffers_binding_descriptions.end());
	binding_descriptions.push_back(instance_buffer_binding_description);

	return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> DrawCall::get_attributes_descriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributes_descriptions;

	auto vertex_buffers_attributes_descriptions = get_vertex_buffers_attributes_descriptions();
	auto instance_buffer_attributes_descriptions = get_instance_buffer_attributes_descriptions();

	attributes_descriptions.insert(attributes_descriptions.begin(), vertex_buffers_attributes_descriptions.begin(), vertex_buffers_attributes_descriptions.end());
	attributes_descriptions.insert(attributes_descriptions.begin(), instance_buffer_attributes_descriptions.begin(), instance_buffer_attributes_descriptions.end());

	return attributes_descriptions;
}

// ------------------------------------------------------------------------------------------------ BakedSceneGraph

void BakedSceneGraph::handle_node_transform_update(Packet const* packet)
{
	auto morphed = static_cast<Packet_SceneGraph_NodeTransformUpdate const*>(packet);
	update_instance(m_draw_calls.at(morphed->m_node->m_geometry), morphed->m_node);
}

void BakedSceneGraph::handle_node_parent_update(Packet const* packet)
{
	auto morphed = static_cast<Packet_SceneGraph_NodeParentUpdate const*>(packet);
	update_instance(m_draw_calls.at(morphed->m_node->m_geometry), morphed->m_node);
}

void BakedSceneGraph::handle_geometry_nodes_update(Packet const* packet)
{
	auto morphed = static_cast<Packet_LinearizedSceneGraph_GeometryNodesUpdate const*>(packet);

	if (morphed->m_added_node && !m_draw_calls.contains(morphed->m_geometry))
	{ // A node has been added and its geometry wasn't uploaded yet.
		recreate_draw_call_for_geometry(morphed->m_geometry);
	}
	else if (morphed->m_removed_node && m_linearized_scene_graph->m_instances_by_geometry.at(morphed->m_geometry).empty())
	{ // All of the instances have been removed from a certain geometry. So also the geometry should be removed.
		destroy_draw_call_of_geometry(morphed->m_geometry);
	}
	else if (morphed->m_added_node || morphed->m_removed_node)
	{ // A node has been added or removed from a certain geometry, the whole instances buffers need to be rebuilt.
		auto draw_call = m_draw_calls.at(morphed->m_geometry);
		realloc_instances_buffers(draw_call, m_linearized_scene_graph->m_instances_by_geometry.at(morphed->m_geometry));
	}
}

void BakedSceneGraph::handle_light_nodes_update(Packet const* packet)
{
	// todo
}

BakedSceneGraph::BakedSceneGraph(LinearizedSceneGraph* linearized_scene_graph) :
	m_linearized_scene_graph(linearized_scene_graph),
	m_packet_handler({
		 { protocol::PacketType::SCENE_GRAPH_NODE_PARENT_UPDATE,    std::bind(&BakedSceneGraph::handle_node_parent_update, this, _1) },
		 { protocol::PacketType::SCENE_GRAPH_NODE_TRANSFORM_UPDATE, std::bind(&BakedSceneGraph::handle_node_transform_update, this, _1) },

		 { protocol::PacketType::LINEARIZED_SCENE_GRAPH_GEOMETRY_NODES_UPDATE, std::bind(&BakedSceneGraph::handle_geometry_nodes_update, this, _1) },
	     { protocol::PacketType::LINEARIZED_SCENE_GRAPH_LIGHT_NODES_UPDATE,    std::bind(&BakedSceneGraph::handle_light_nodes_update, this, _1) }
	})
{
	init();
}

void BakedSceneGraph::destroy_draw_call_of_geometry(Geometry const* geometry)
{
	if (m_draw_calls.contains(geometry))
	{
		DrawCall& draw_call = m_draw_calls.at(geometry);
		destroy_vertex_buffers(draw_call);
		destroy_indices_buffer(draw_call);
		destroy_instances_buffers(draw_call);

		m_draw_calls.erase(geometry);
	}
}

void BakedSceneGraph::recreate_draw_call_for_geometry(Geometry const* geometry)
{
	if (!m_draw_calls.contains(geometry)) {
		m_draw_calls.emplace(geometry, DrawCall{});
	}
	DrawCall& draw_call = m_draw_calls.at(geometry);
	realloc_vertex_buffers(draw_call, geometry);
	realloc_indices_buffer(draw_call, geometry);
	realloc_instances_buffers(draw_call, m_linearized_scene_graph->m_instances_by_geometry.at(geometry));
}

void BakedSceneGraph::destroy_vertex_buffers(DrawCall& draw_call)
{
	for (VertexBuffer vertex_buffer : draw_call.m_vertex_buffers) {
		m_gpu_allocator.destroy_buffer_if_any(vertex_buffer);
	}

	draw_call.m_vertex_buffers.clear();
}

void BakedSceneGraph::realloc_vertex_buffers(DrawCall& draw_call, Geometry const* geometry)
{
	destroy_vertex_buffers(draw_call);

	for (size_t attrib_idx = 0; attrib_idx < AttribType::PerVertexAttribCount; attrib_idx++)
	{
		AttribType::Enum attrib_type = static_cast<AttribType::Enum>(attrib_idx);

		auto accessor = geometry->m_attributes[attrib_type];
		VertexBuffer& vertex_buffer = draw_call.m_vertex_buffers[attrib_type];

		m_gpu_allocator.alloc_device_only_buffer(vertex_buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, accessor->get_size());
		m_gpu_allocator.update_device_only_buffer(vertex_buffer, accessor->get_data(), accessor->get_size(), 0);
	}

	if (m_recording)
	{
		m_packet_pool.add_packet(new Packet_BakedSceneGraph_RebuiltVertexBuffers());
	}
}

void BakedSceneGraph::destroy_indices_buffer(DrawCall& draw_call)
{
	m_gpu_allocator.destroy_buffer_if_any(draw_call.m_indices_buffer.m_buffer);
	draw_call.m_indices_buffer.m_indices_count = 0;
}

void BakedSceneGraph::realloc_indices_buffer(DrawCall& draw_call, Geometry const* geometry)
{
	destroy_indices_buffer(draw_call);

	if (auto accessor = geometry->m_indices)
	{
		IndicesBuffer& indices_buffer = draw_call.m_indices_buffer;

		m_gpu_allocator.alloc_device_only_buffer(indices_buffer.m_buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, accessor->get_size());
		m_gpu_allocator.update_device_only_buffer(indices_buffer.m_buffer, accessor->get_data(), accessor->get_size(), 0);

		indices_buffer.m_indices_count = accessor->m_count;
	}

	if (m_recording)
	{
		m_packet_pool.add_packet(new Packet_BakedSceneGraph_RebuiltIndicesBuffer());
	}
}

void BakedSceneGraph::destroy_instances_buffers(DrawCall& draw_call)
{
	for (InstanceBuffer& instance_buffer : draw_call.m_instances_buffers) {
		m_gpu_allocator.destroy_buffer_if_any(instance_buffer.m_buffer);
	}

	draw_call.m_instances_buffers.fill({});
}

void BakedSceneGraph::realloc_instances_buffers(DrawCall& draw_call, std::unordered_set<Node const*> const& instances)
{
	destroy_instances_buffers(draw_call);

	constexpr size_t instance_data_size = 16 * sizeof(float);
	constexpr size_t buffers_count = static_cast<size_t>(AllocationType::Count);

	const size_t max_capacity = instances.size() * instance_data_size;

	std::array<std::vector<uint8_t>, buffers_count> staging_buffers;
	std::array<size_t, buffers_count> data_offsets;

	for (Node const* instance : instances)
	{
		auto allocation_type_ptr = scene_graph::g_entt_registry.try_get<AllocationType::Enum>(instance->m_entity);
		auto allocation_type = allocation_type_ptr ? *allocation_type_ptr : AllocationType::STATIC;

		size_t buffer_idx = allocation_type;

		std::vector<uint8_t>& staging_buffer = staging_buffers[buffer_idx];
		if (staging_buffer.capacity() < max_capacity) {
			staging_buffer.resize(max_capacity);
		}

		size_t& data_offset = data_offsets[buffer_idx];

		std::memcpy(staging_buffer.data() + data_offset, instance->m_world_transform, data_offset);

		InstanceBuffer& instance_buffer = draw_call.m_instances_buffers[buffer_idx];
		instance_buffer.m_offset_by_instance.emplace(instance, data_offset);
		instance_buffer.m_instances_count++;

		data_offset += instance_data_size;
	}

	for (size_t allocation_type = 0; allocation_type < static_cast<size_t>(AllocationType::Count); allocation_type++)
	{
		InstanceBuffer& instance_buffer = draw_call.m_instances_buffers[allocation_type];

		uint8_t* staging_buffer = staging_buffers[allocation_type].data();
		size_t data_size = data_offsets[allocation_type];
		if (data_size == 0) {
			continue;
		}

		switch (allocation_type)
		{
		case AllocationType::FREQUENTLY_UPDATED:
			m_gpu_allocator.alloc_host_visible_buffer(instance_buffer.m_buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data_size);
			m_gpu_allocator.update_host_visible_buffer(instance_buffer.m_buffer, staging_buffer, data_size, 0);
			break;

		default:
			printf("Illegal AllocationType: `%lld`. Assuming: `STATIC`.\n", allocation_type);

		case AllocationType::STATIC:
			m_gpu_allocator.alloc_device_only_buffer(instance_buffer.m_buffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, data_size);
			m_gpu_allocator.update_device_only_buffer(instance_buffer.m_buffer, staging_buffer, data_size, 0);
			break;
		}
	}

	if (m_recording)
	{
		m_packet_pool.add_packet(new Packet_BakedSceneGraph_RebuiltInstancesBuffer());
	}
}

void BakedSceneGraph::update_instance(DrawCall& draw_call, Node const* node)
{
	for (size_t buffer_type = 0; buffer_type < AllocationType::Enum::Count; buffer_type++)
	{
		InstanceBuffer& instance_buffer = draw_call.m_instances_buffers.at(buffer_type);

		if (instance_buffer.m_offset_by_instance.contains(node))
		{
			size_t offset = instance_buffer.m_offset_by_instance.at(node);
			size_t size = 16 * sizeof(float); // instance_data_size

			std::vector<uint8_t> data;
			data.resize(size);

			std::memcpy(data.data(), node->m_world_transform, 16 * sizeof(float));
			m_gpu_allocator.update_buffer(instance_buffer.m_buffer, data.data(), size, offset);
			return;
		}
	}

	printf("Couldn't find the node `%s` (ID=%d). It's not baked here.\n", node->m_name.c_str(), node->m_entity);
}

void BakedSceneGraph::init()
{
	m_recording = false;

	for (auto [geometry, _] : m_linearized_scene_graph->m_instances_by_geometry) {
		recreate_draw_call_for_geometry(geometry);
	}

	m_recording = true;
}

void BakedSceneGraph::apply_changes()
{
	m_linearized_scene_graph->m_packet_pool.handle(g_packet_handler_class, m_packet_handler);
}

