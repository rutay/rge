#include "scene.hpp"

#include <bx/math.h>

using namespace rge;

size_t rge::ComponentType_byte_size(ComponentType component_type)
{
	switch (component_type)
	{
	case ComponentType::BYTE:           return sizeof(char);
	case ComponentType::UNSIGNED_BYTE:  return sizeof(unsigned char);
	case ComponentType::SHORT:          return sizeof(short);
	case ComponentType::UNSIGNED_SHORT: return sizeof(unsigned short);
	case ComponentType::FLOAT:          return sizeof(float);
	case ComponentType::UNSIGNED_INT:   return sizeof(unsigned int);
	default:
		// todo errorrrr
		return 0;
	}
}

// ----------------------------------------------------------

Node::Node() :
	m_parent(nullptr),

	m_meshes(),
	m_light(nullptr),

	m_position{0, 0, 0},
	m_scale{1, 1, 1},
	m_rotation{0, 0, 0, 1},

	m_local_transform{},
	m_world_transform{}
{
	bx::mtxIdentity(m_local_transform);
	bx::mtxIdentity(m_world_transform);
}

bool Node::is_orphan() const
{
    return m_parent;
}

bool Node::is_parent() const
{
    return !m_children.empty();
}

void Node::update_position_rotation_scale_from_local_transform()
{
	m_position = {
		.x = m_local_transform[12],
		.y = m_local_transform[13],
		.z = m_local_transform[14]
	};
	m_rotation = { }; // todo
	m_scale = {
		.x = m_local_transform[0],
		.y = m_local_transform[5],
		.z = m_local_transform[10]
	};
}

void Node::update_local_transform()
{
	float scale_mtx[16];
	float rot_mtx[16];
	float translation_mtx[16];

	bx::mtxScale(scale_mtx, m_scale.x, m_scale.y, m_scale.z);
	bx::mtxQuat(rot_mtx, bx::Quaternion{m_rotation.x, m_rotation.y, m_rotation.z, m_rotation.w});
	bx::mtxTranslate(translation_mtx, m_position.x, m_position.y, m_position.z);

	float rot_scale_mtx[16];
	bx::mtxMul(rot_scale_mtx, rot_mtx, scale_mtx);
	bx::mtxMul(m_local_transform, translation_mtx, rot_scale_mtx);
}

void Node::update_world_transform()
{
	bx::memCopy(m_world_transform, m_local_transform, sizeof(m_local_transform));

	Node* parent = m_parent;
	while (parent != nullptr)
	{
		float world_transform_mtx[16];
		bx::mtxMul(world_transform_mtx, m_world_transform, parent->m_local_transform);
		bx::memCopy(m_world_transform, world_transform_mtx, sizeof(world_transform_mtx));

		parent = parent->m_parent;
	}
}

void Node::traverse(std::function<void(Node* node)> on_node)
{
    on_node(this);

    for (Node* child : m_children)
        child->traverse(on_node);
}

void Node::traverse_const(std::function<void(Node const* node)> on_node) const
{
    on_node(this);

    for (Node* child : m_children)
        child->traverse_const(on_node);
}
