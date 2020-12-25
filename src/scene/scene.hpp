#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include <bx/math.h>
#include <bgfx/bgfx.h>

namespace rge
{
    struct Material
    {
        float m_albedo[3];
        float m_metallic;
        float m_roughness;
        float m_ao;
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

    enum struct ComponentType
    {
        BYTE           = 5120,
        UNSIGNED_BYTE  = 5121,
        SHORT          = 5122,
        UNSIGNED_SHORT = 5123,
        UNSIGNED_INT   = 5125,
        FLOAT          = 5126
    };

    struct Buffer
    {
        ComponentType m_component_type;
        size_t m_num_components;
        size_t m_count;

        size_t m_stride;
        std::vector<uint8_t> m_data;

        bool m_normalized;
    };

    enum AttributeType
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

        Buffer* m_attributes[AttributeType::Count];
        Buffer* m_indices;
    };

    struct Node
    {
        Node* m_parent;

        Mesh* m_mesh;
        Light* m_light;

        float m_position[3];
        float m_rotation[3];
        float m_scale[3];

        std::vector<Node*> m_children;
    };
}