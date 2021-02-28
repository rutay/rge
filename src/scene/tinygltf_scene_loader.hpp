#pragma once

#include "scene.hpp"

#include <tiny_gltf.h>

namespace rge::scene
{
struct SceneLoader_tinygltf
{
private:
	std::unordered_map<int, Buffer*> m_buffer_by_idx;
	std::unordered_map<int, BufferView*> m_buffer_view_by_idx;
	std::unordered_map<int, Accessor*> m_accessor_by_idx;
	std::unordered_map<int, Material*> m_material_by_idx;
	std::unordered_map<int, Mesh*> m_mesh_by_idx;
	std::unordered_map<int, Node*> m_node_by_idx;
	std::unordered_map<int, Node*> m_scene_by_idx;

public:
	Buffer* load_buffer(tinygltf::Model const& gltf_model, int buffer_idx);
	BufferView* load_buffer_view(tinygltf::Model const& gltf_model, int buffer_view_idx);
	Accessor* load_accessor(tinygltf::Model const& gltf_model, int accessor_idx);
	Material* load_material(tinygltf::Model const& gltf_model, int material_idx);
	Mesh* load_primitive(tinygltf::Model const& gltf_model, int mesh_idx, tinygltf::Primitive const& gltf_primitive);
	void load_mesh(tinygltf::Model const& gltf_model, int mesh_idx, std::vector<Mesh*>& meshes);
	Node* load_node(tinygltf::Model const& gltf_model, int node_idx, Node* parent);
	Node* load_scene(tinygltf::Model const& gltf_model, int scene_idx);

	void clear_cache();
	Node* load(tinygltf::Model& gltf_model);
	Node* load_from_resource(char const* path);
};
}

