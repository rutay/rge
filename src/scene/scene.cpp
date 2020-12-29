#include "scene.hpp"

using namespace rge;

bool Node::is_orphan()
{
    return m_parent;
}

bool Node::is_parent()
{
    return m_children.size() > 0;
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
