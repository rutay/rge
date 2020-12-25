#include "node.hpp"

#include <bx/bx.h>

using namespace rge;

NodePacket::NodePacket(PacketType type, Node* node) :
    Packet(type),
    m_node(node)
{}

// ------------------------------------------------------------------------------------------------

inline bool Node::is_orphan()
{
    return m_parent == NULL || m_child_idx < 0;
}

inline bool Node::is_parent()
{
    return m_children.size() > 0;
}

void Node::set_mesh(Mesh* mesh, RGE_PACKET_BUFFERS_DEF(packet_buffers))
{
    if (m_mesh == mesh) return;

    m_mesh = mesh;

    if (packet_buffers)
    {
        NodePacket packet(RGE_PROTOCOL_NODE_SET_MESH, this);
        packet.m_mesh = mesh;
        RGE_PACKET_BUFFERS_ADD_PACKET(packet_buffers, packet);
    }
}

void Node::set_light(Light* light, RGE_PACKET_BUFFERS_DEF(packet_buffers))
{
    if (m_light == light) return;

    m_light = light;

    if (packet_buffers)
    {
        NodePacket packet(RGE_PROTOCOL_NODE_SET_LIGHT, this);
        packet.m_light = light;
        RGE_PACKET_BUFFERS_ADD_PACKET(packet_buffers, packet);
    }
}

void Node::set_transform(float transform[16], RGE_PACKET_BUFFERS_DEF(packet_buffers))
{
    bx::memCopy(m_transform, transform, sizeof(float) * 16);
    // TODO m_world_transform

    if (packet_buffers)
    {
        NodePacket packet(RGE_PROTOCOL_NODE_SET_TRANSFORM, this);
        bx::memCopy(packet.m_transform, transform, sizeof(float) * 16);
        RGE_PACKET_BUFFERS_ADD_PACKET(packet_buffers, packet);
    }
}

void Node::add_child(Node* node, RGE_PACKET_BUFFERS_DEF(packet_buffers))
{
    if (node->m_parent)
    {
        // The given node already has a parent, scene graph should be direct. <-

        node->m_parent->remove_child(node, packet_buffers, packet_buffers_size);
    }

    node->m_parent = this;
    node->m_child_idx = m_children.size();

    m_children.push_back(node);

    if (packet_buffers)
    {
        NodePacket packet(RGE_PROTOCOL_NODE_ADD_CHILD, this);
        packet.m_added_child_node = node;
        RGE_PACKET_BUFFERS_ADD_PACKET(packet_buffers, packet);
    }
}

void Node::remove_child(Node* node, RGE_PACKET_BUFFERS_DEF(packet_buffers))
{
    if (node->m_parent != this) return;

    m_children.erase(m_children.begin() + node->m_child_idx);

    for (int idx = 0; idx < m_children.size(); idx++)
        m_children[idx]->m_child_idx = idx;

    if (packet_buffers)
    {
        NodePacket packet(RGE_PROTOCOL_NODE_REMOVE_CHILD, this);
        packet.m_removed_child_node = node;
        RGE_PACKET_BUFFERS_ADD_PACKET(packet_buffers, packet);
    }
}

void Node::remove_if(Node* node, std::function<bool(Node*)> callback)
{
    remove_if(node, callback);


}

void Node::traverse(Node* node, std::function<void(Node*)> callback)
{
    callback(node);

    for (Node* child : node->m_children)
        callback(child);
}
