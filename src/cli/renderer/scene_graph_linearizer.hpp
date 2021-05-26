#pragma once

#include "scene/scene.hpp"
#include "pipeline.hpp"

#include <unordered_set>
#include <unordered_map>

namespace rge
{
	// ------------------------------------------------------------------------------------------------ scene_graph_linearizer

	class scene_graph_linearizer :
		public rge::pass<Node*, rge::scene_graph_linearizer*>
	{
	public:
		static PacketHandlerClass s_packet_handler_class;

		static void init();

	private:
		/* SceneGraph */
		void handle_geometry_update(Packet const* packet);
		void handle_light_update(Packet const* packet);
		void handle_node_set_geometry(Packet const* packet);
		void handle_node_set_material(Packet const* packet);
		void handle_node_set_light(Packet const* packet);
		void handle_node_transform_update(Packet const* packet);
		void handle_node_parent_update(Packet const* packet);

		void init0();

	public:
		PacketPool m_packet_pool;
		MultiplexerPacketHandler const m_packet_handler;

		// Result
		std::unordered_set<Node const*> m_nodes;
		std::unordered_map<Geometry const*, std::unordered_set<Node const*>> m_instances_by_geometry;
		std::unordered_set<Node const*> m_light_nodes;


		scene_graph_linearizer(Node* scene_graph);

		void add_node_for_geometry(Geometry* geometry, Node const* node, bool log = true);
		void remove_node_from_geometry(Geometry* geometry, Node const* node, bool log = true);
		void add_light_node(Node const* light_node, bool log = true);
		void remove_light_node(Node const* light_node, bool log = true);

		void apply_changes();
	};

	// ------------------------------------------------------------------------------------------------ Packets

	struct Packet_LinearizedSceneGraph_GeometryNodesUpdate :
		public define_packet<protocol::PacketType::LINEARIZED_SCENE_GRAPH_GEOMETRY_NODES_UPDATE>
	{
		Geometry const* m_geometry;
		Node const* m_added_node;
		Node const* m_removed_node;
	};

	struct Packet_LinearizedSceneGraph_LightNodesUpdate :
		public define_packet<protocol::PacketType::LINEARIZED_SCENE_GRAPH_LIGHT_NODES_UPDATE>
	{
		Node const* m_added_light_node;
		Node const* m_removed_light_node;
	};
}
