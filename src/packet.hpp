#pragma once

#define RGE_PROTOCOL_NODE               0x00010000u
#define RGE_PROTOCOL_NODE_ADD_CHILD     0x00010000u
#define RGE_PROTOCOL_NODE_REMOVE_CHILD  0x00010001u
#define RGE_PROTOCOL_NODE_SET_TRANSFORM 0x00010002u
#define RGE_PROTOCOL_NODE_SET_MESH      0x00010003u
#define RGE_PROTOCOL_NODE_SET_LIGHT     0x00010004u


namespace rge
{
    using PacketType = uint32_t;

    struct Packet
    {
        PacketType const m_type;

        Packet(PacketType type) : m_type(type) {}
    };

    using PacketBuffer = std::vector<Packet const*>;

    struct MultiTargetPacketHandler
    {
        Packet m_packet;
        // m_state

        bool has_processed();
    };

    namespace PacketPool
    {
    };
}

