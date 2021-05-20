#pragma once

#include "scene_graph_linearizer.hpp"
#include "gpu_allocator.hpp"

#include <entt/entt.hpp>

namespace rge
{
	// ------------------------------------------------------------------------------------------------ scene_graph_baker

	namespace scene_graph_baker
	{
		PacketHandlerClass g_packet_handler_class;

		void init();
	}

	// ------------------------------------------------------------------------------------------------ BakedSceneGraph

	struct AllocationType
	{
		enum Enum
		{
			FREQUENTLY_UPDATED,
			STATIC,

			Count
		};
	};

	using VertexBuffer = GpuBuffer;

	struct IndicesBuffer
	{
		GpuBuffer m_buffer;
		size_t m_indices_count;
		VkIndexType m_index_type = VK_INDEX_TYPE_UINT32;
	};

	struct InstanceBuffer
	{
		GpuBuffer m_buffer;
		std::unordered_map<Node const*, size_t> m_offset_by_instance;
		size_t m_instances_count;

		bool is_allocated();
	};

	struct DrawCall
	{
		std::vector<VertexBuffer> m_vertex_buffers;

		IndicesBuffer m_indices_buffer;

		std::array<InstanceBuffer, AllocationType::Count> m_instances_buffers;

		// TODO constexpr

		static std::vector<VkVertexInputBindingDescription> get_vertex_buffers_binding_descriptions();
		static std::vector<VkVertexInputAttributeDescription> get_vertex_buffers_attributes_descriptions();

		static VkVertexInputBindingDescription get_instance_buffer_binding_description();
		static std::vector<VkVertexInputAttributeDescription> get_instance_buffer_attributes_descriptions();

		static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
		static std::vector<VkVertexInputAttributeDescription> get_attributes_descriptions();
	};

	class BakedSceneGraph
	{
	private:
		MultiplexerPacketHandler const m_packet_handler;

		/* SceneGraph */
		void handle_node_transform_update(Packet const* packet);
		void handle_node_parent_update(Packet const* packet);

		/* LinearizedSceneGraph */
		void handle_geometry_nodes_update(Packet const* packet);
		void handle_light_nodes_update(Packet const* packet);

		void init();

	public:
		LinearizedSceneGraph* const m_linearized_scene_graph;

		bool m_recording = true;
		PacketPool m_packet_pool;

		GpuAllocator m_gpu_allocator;

		/* Result */
		std::unordered_map<Geometry const*, DrawCall> m_draw_calls;

		/**/

		BakedSceneGraph(LinearizedSceneGraph* linearized_scene_graph);

		void destroy_draw_call_of_geometry(Geometry const* geometry);
		void recreate_draw_call_for_geometry(Geometry const* geometry);

		void destroy_vertex_buffers(DrawCall& draw_call);
		void realloc_vertex_buffers(DrawCall& draw_call, Geometry const* geometry);

		void destroy_indices_buffer(DrawCall& draw_call);
		void realloc_indices_buffer(DrawCall& draw_call, Geometry const* geometry);

		void destroy_instances_buffers(DrawCall& draw_call);
		void realloc_instances_buffers(DrawCall& draw_call, std::unordered_set<Node const*> const& instances);

		void update_instance(DrawCall& draw_call, Node const* node);

		void apply_changes();
	};

	// ------------------------------------------------------------------------------------------------ Packets

	using Packet_BakedSceneGraph_RebuiltVertexBuffers   = define_packet<protocol::PacketType::BAKED_SCENE_GRAPH_REBUILT_VERTEX_BUFFERS>;
	using Packet_BakedSceneGraph_RebuiltIndicesBuffer   = define_packet<protocol::PacketType::BAKED_SCENE_GRAPH_REBUILT_INDICES_BUFFER>;
	using Packet_BakedSceneGraph_RebuiltInstancesBuffer = define_packet<protocol::PacketType::BAKED_SCENE_GRAPH_REBUILT_INSTANCES_BUFFERS>;
}
