#pragma once

#include "material.hpp"

#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <optional>
#include <array>

#include <bgfx/bgfx.h>

namespace rge
{
enum ComponentType
{
	BYTE = 5120,
	UNSIGNED_BYTE = 5121,
	SHORT = 5122,
	UNSIGNED_SHORT = 5123,
	UNSIGNED_INT = 5125,
	FLOAT = 5126
};

size_t ComponentType_byte_size(ComponentType component_type);

enum class Type
{
	VEC2 = 2,
	VEC3 = 3,
	VEC4 = 4,
	MAT2 = 32 + 2,
	MAT3 = 32 + 3,
	MAT4 = 32 + 4,
	SCALAR = 64 + 1,
	VECTOR = 64 + 4,
	MATRIX = 64 + 16
};

struct Buffer
{
	void* m_data;
	size_t m_byte_length;
};

struct BufferView
{
	Buffer* m_buffer;

	size_t m_byte_offset;
	size_t m_byte_length;
	size_t m_byte_stride;
};

struct Accessor
{
	BufferView* m_buffer_view;

	//Type m_type;
	ComponentType m_component_type;
	size_t m_num_components;

	size_t m_byte_offset;
	size_t m_count;

	bool m_normalized;

	inline size_t get_value_byte_size() const
	{
		return ComponentType_byte_size(m_component_type) * m_num_components;
	}

	inline size_t get_stride() const
	{
		return m_buffer_view->m_byte_stride > 0 ? m_buffer_view->m_byte_stride : get_value_byte_size();
	}
};

enum LightType
{
	POINT,
	DIRECTIONAL,
	SPOT,

	Count
};

struct Light
{
	LightType m_type;
	Vec3 m_color;
	float m_intensity;
};

struct PointLight: public Light
{
	float m_radius;
};

struct DirectionalLight: public Light
{
	Vec3 m_direction;
};

struct SpotLight: public Light
{
	Vec3 m_direction;
	float m_angle;
};

struct AttribType
{
	enum Enum
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
};

struct DrawMode
{
	enum Enum
	{
		POINTS,
		LINES,
		LINE_LOOP,
		LINE_STRIP,
		TRIANGLES,
		TRIANGLE_STRIP,
		TRIANGLE_FAN,

		Count
	};
};

struct Geometry
{
	DrawMode::Enum m_mode;

	Accessor* m_attributes[static_cast<size_t>(AttribType::Count)];
	Accessor* m_indices;
};

struct Mesh
{
	Material* m_material;
	Geometry* m_geometry;
};

struct Node
{
	Node();

	Node* m_parent;

	std::vector<Mesh*> m_meshes;
	Light* m_light;

	Vec3 m_position;
	Quaternion m_rotation;
	Vec3 m_scale;

	float m_local_transform[16];
	float m_world_transform[16];

	void update_position_rotation_scale_from_local_transform();
	void update_local_transform();
	void update_world_transform();

	std::vector<Node*> m_children;

	bool is_orphan() const;
	bool is_parent() const;

	void traverse(std::function<void(Node* node)> on_node);
	void traverse_const(std::function<void(Node const* node)> on_node) const;
};
}