#include "scene.hpp"

#include <bx/math.h>

using namespace rge::scene;

size_t rge::scene::ComponentType_byte_size(ComponentType component_type)
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

void Node::update_local_transform()
{
	float tmp[16];

	bx::mtxIdentity(m_local_transform);

	bx::mtxQuat(tmp, bx::Quaternion{m_rotation[0], m_rotation[1], m_rotation[2], m_rotation[3]});
	bx::mtxMul(m_local_transform, tmp, m_local_transform);

	bx::mtxScale(tmp, m_scale[0], m_scale[1], m_scale[2]);
	bx::mtxMul(m_local_transform, tmp, m_local_transform);

	bx::mtxTranslate(tmp, m_position[0], m_position[1], m_position[2]);
	bx::mtxMul(m_local_transform, tmp, m_local_transform);
}

void Node::update_world_transform()
{
	bx::memCopy(m_world_transform, m_local_transform, 16 * sizeof(float));

	Node* node = this;
	while (node != nullptr) {
		// TODO (OPT) do not multiply if the parent's transform is an identity?
		bx::mtxMul(m_world_transform, node->m_local_transform, m_world_transform);

		node = node->m_parent;
	}
}

void Node::traverse(std::function<void(Node* node)> on_node)
{
    on_node(this);

    for (Node* child : m_children)
    {
        child->traverse(on_node);
    }
}

void Node::traverse_const(std::function<void(Node const* node)> on_node) const
{
    on_node(this);

    for (Node* child : m_children)
    {
        child->traverse_const(on_node);
    }
}
