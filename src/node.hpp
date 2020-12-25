#pragma once

#include <vector>

#include "packet.hpp"

namespace rge
{
    struct Node;

    struct NodePacket : public Packet
    {
        Node* m_node;

        // RGE_PROTOCOL_NODE_SET_MESH
        Mesh* m_old_mesh;
        Mesh* m_mesh;

        Light* m_light;

        float m_transform[16];

        Node* m_added_child_node;
        Node* m_removed_child_node;

        uint64_t m_state;

        NodePacket(PacketType type, Node* node);
    };

    struct Node
    {
        Node* m_parent;
        int m_child_idx;

        Mesh* m_mesh;
        Light* m_light;

        float m_transform[16];
        float m_world_transform[16];

        std::vector<Node*> m_children;

        bool is_orphan();
        bool is_parent();

        void set_mesh(Mesh* mesh, RGE_PACKET_BUFFERS_DEF(packet_buffers));
        void set_light(Light* light, RGE_PACKET_BUFFERS_DEF(packet_buffers));
        void set_transform(float transform[16], RGE_PACKET_BUFFERS_DEF(packet_buffers));

        void add_child(Node* node, RGE_PACKET_BUFFERS_DEF(packet_buffers));
        void remove_child(Node* node, RGE_PACKET_BUFFERS_DEF(packet_buffers));

        static void remove_if(Node* node, std::function<bool(Node*)> callback);
        static void traverse(Node* node, std::function<void(Node*)> callback);
    };
}

