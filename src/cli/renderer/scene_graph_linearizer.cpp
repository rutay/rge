
#include "scene_graph_linearizer.hpp"

using namespace rge;

using namespace std::placeholders;

// ------------------------------------------------------------------------------------------------ scene_graph_linearizer

void scene_graph_linearizer::init()
{
	packet_handler_class_registry::register_packet_handler_class(s_packet_handler_class, "scene_graph_linearizer_packet_handler");
}

// ------------------------------------------------------------------------------------------------ LinearizedSceneGraph

void scene_graph_linearizer::handle_geometry_update(Packet const* packet)
{}

void scene_graph_linearizer::handle_light_update(Packet const* packet)
{}

void scene_graph_linearizer::handle_node_set_geometry(Packet const* packet)
{
	auto morphed_packet = morph_packet_const<Packet_SceneGraph_NodeSetGeometry>(packet);

	if (morphed_packet->m_old_geometry) {
		remove_node_from_geometry(morphed_packet->m_old_geometry, morphed_packet->m_node);

		// Remove the geometry if it has no nodes left, is it worth?
	}

	if (morphed_packet->m_new_geometry) {
		add_node_for_geometry(morphed_packet->m_new_geometry, morphed_packet->m_node);
	}
}

void scene_graph_linearizer::handle_node_set_material(Packet const* packet)
{
	// todo
}

void scene_graph_linearizer::handle_node_set_light(Packet const* packet)
{
	auto morphed_packet = morph_packet_const<Packet_SceneGraph_NodeSetLight>(packet);

	morphed_packet->m_light ? add_light_node(morphed_packet->m_node) : remove_light_node(morphed_packet->m_node);
}

void scene_graph_linearizer::handle_node_transform_update(Packet const* packet)
{}

void scene_graph_linearizer::handle_node_parent_update(Packet const* packet)
{
	auto morphed_packet = morph_packet_const<Packet_SceneGraph_NodeParentUpdate>(packet);

	auto [_, inserted] = m_nodes.insert(morphed_packet->m_node);
	if (inserted && morphed_packet->m_node->m_geometry) {
		add_node_for_geometry(morphed_packet->m_node->m_geometry, morphed_packet->m_node);
	}
}

scene_graph_linearizer::scene_graph_linearizer(Node* scene_graph) : rge::pass<Node*, rge::scene_graph_linearizer*>(scene_graph),
	m_packet_handler({
		 { protocol::PacketType::SCENE_GRAPH_GEOMETRY_UPDATE,       std::bind(&scene_graph_linearizer::handle_geometry_update, this, _1) },
		 { protocol::PacketType::SCENE_GRAPH_LIGHT_UPDATE,          std::bind(&scene_graph_linearizer::handle_light_update, this, _1) },
		 { protocol::PacketType::SCENE_GRAPH_NODE_SET_GEOMETRY,     std::bind(&scene_graph_linearizer::handle_node_set_geometry, this, _1) },
		 { protocol::PacketType::SCENE_GRAPH_NODE_SET_MATERIAL,     std::bind(&scene_graph_linearizer::handle_node_set_material, this, _1) },
		 { protocol::PacketType::SCENE_GRAPH_NODE_SET_LIGHT,        std::bind(&scene_graph_linearizer::handle_node_set_light, this, _1) },
		 { protocol::PacketType::SCENE_GRAPH_NODE_TRANSFORM_UPDATE, std::bind(&scene_graph_linearizer::handle_node_transform_update, this, _1) },
		 { protocol::PacketType::SCENE_GRAPH_NODE_PARENT_UPDATE,    std::bind(&scene_graph_linearizer::handle_node_parent_update, this, _1) }
	})
{
	this->init0();
}

void scene_graph_linearizer::add_node_for_geometry(Geometry* geometry, Node const* node, bool log)
{
	if (!m_instances_by_geometry.contains(geometry)) {
		m_instances_by_geometry.emplace(geometry, std::unordered_set<Node const*>());
	}
	m_instances_by_geometry[geometry].insert(node);

	if (log)
	{
		Packet_LinearizedSceneGraph_GeometryNodesUpdate* packet = new Packet_LinearizedSceneGraph_GeometryNodesUpdate();
		packet->m_geometry = geometry;
		packet->m_added_node = node;
		packet->m_removed_node = nullptr;
		m_packet_pool.add_packet(packet);
	}
}

void scene_graph_linearizer::remove_node_from_geometry(Geometry* geometry, Node const* node, bool log)
{
	if (m_instances_by_geometry.contains(geometry))
	{
		if (m_instances_by_geometry.at(geometry).erase(node) > 0 && log)
		{
			Packet_LinearizedSceneGraph_GeometryNodesUpdate* packet = new Packet_LinearizedSceneGraph_GeometryNodesUpdate;
			packet->m_geometry = geometry;
			packet->m_added_node = nullptr;
			packet->m_removed_node = node;
			m_packet_pool.add_packet(packet);
		}
	}
}

void scene_graph_linearizer::add_light_node(Node const* light_node, bool log)
{
	m_light_nodes.insert(light_node);

	if (log)
	{
		Packet_LinearizedSceneGraph_LightNodesUpdate* packet = new Packet_LinearizedSceneGraph_LightNodesUpdate;
		packet->m_added_light_node = light_node;
		packet->m_removed_light_node = nullptr;
		m_packet_pool.add_packet(packet);
	}
}

void scene_graph_linearizer::remove_light_node(Node const* light_node, bool log)
{
	m_light_nodes.erase(light_node);

	if (log)
	{
		Packet_LinearizedSceneGraph_LightNodesUpdate* packet = new Packet_LinearizedSceneGraph_LightNodesUpdate;
		packet->m_added_light_node = nullptr;
		packet->m_removed_light_node = light_node;
		m_packet_pool.add_packet(packet);
	}
}

void scene_graph_linearizer::init0()
{
	printf("Initializing linearized SceneGraph: `%s`...\n", m_input->m_name.c_str());

	m_input->traverse_const([this](Node const* node) {
		if (node->m_light) {
			add_light_node(node, false);
		}

		if (node->m_geometry) {
			add_node_for_geometry(node->m_geometry, node, false);
		}
	});
}

void scene_graph_linearizer::apply_changes()
{
	m_input->handle_packets_deeply(s_packet_handler_class, m_packet_handler);
}
