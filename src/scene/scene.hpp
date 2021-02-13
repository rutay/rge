#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <optional>
#include <array>

#include <bgfx/bgfx.h>

namespace rge
{
    enum struct ComponentType
    {
        BYTE = 5120,
        UNSIGNED_BYTE = 5121,
        SHORT = 5122,
        UNSIGNED_SHORT = 5123,
        UNSIGNED_INT = 5125,
        FLOAT = 5126
    };

    struct AccessorBuffer
    {
        ComponentType m_component_type;
        size_t m_component_size;
        size_t m_num_components;

        size_t m_count;

        size_t m_stride;
        std::vector<uint8_t> m_data;

        bool m_normalized;

        size_t get_data_size() const;
    };

    enum class LightType
    {
        POINT,
        DIRECTIONAL,
        SPOT,

        Count
    };

    struct Light
    {
        LightType const m_type;

        float color[3];
        float intensity;

        bool m_dirty;

    protected:
        Light(LightType type) : m_type(type) {}
    };

    struct PointLight : public Light
    {
        PointLight() : Light(LightType::POINT) {}
    };

    struct DirectionalLight : public Light
    {
        DirectionalLight() : Light(LightType::DIRECTIONAL) {}
    };

    struct SpotLight : public Light
    {
        // TODO

        SpotLight() : Light(LightType::SPOT) {}
    };

    struct Material
    {
        float m_albedo[3];
        float m_metallic;
        float m_roughness;
        float m_ao;
    };

    enum AttribType
    {
        POSITION,
        NORMAL,
        TANGENT,
        TEXCOORD_0,
        TEXCOORD_1,
        COLOR_0,
        JOINTS_0,
        WEIGHTS_0,

        Count
    };

    struct Mesh
    {
        Material* m_material;

        AccessorBuffer* m_attributes[AttribType::Count];
        AccessorBuffer* m_indices;
    };

    struct Node
    {
    	Node();

    	Node* m_parent;

        Mesh* m_mesh;
        Light* m_light;

		float m_rotation[4];
		float m_position[3];
		float m_scale[3];

		float m_local_transform[16];
		float m_world_transform[16];

        void update_local_transform();
        void update_world_transform();

        std::vector<Node*> m_children;

        bool is_orphan() const;
        bool is_parent() const;

        void traverse(std::function<void(Node* node)> on_node);
        void traverse_const(std::function<void(Node const* node)> on_node) const;
    };
}