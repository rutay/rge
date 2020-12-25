#pragma once

#include "scene.hpp"

#include <tiny_gltf.h>

namespace rge
{
    struct GltfSceneLoader
    {
    private:
        std::unordered_map<int, Buffer*>   m_buffer_by_accessor_idx;
        std::unordered_map<int, Material*> m_material_by_idx;
        std::unordered_map<int, Mesh*>     m_mesh_by_idx;
        std::unordered_map<int, Node*>     m_node_by_idx;
        std::unordered_map<int, Node*>     m_scene_by_idx;

    public:
        Buffer* create_buffer_from_accessor(tinygltf::Model& gltf_model, int accessor_idx);
        Material* load_material(tinygltf::Model& gltf_model, int material_idx);
        Mesh* load_mesh(tinygltf::Model& gltf_model, int mesh_idx);
        Node* load_node(tinygltf::Model& gltf_model, int node_idx, Node* parent);
        Node* load_scene(tinygltf::Model& gltf_model, int scene_idx);

        void clear_cache();
        void print_cache();

        Node* load(tinygltf::Model& gltf_model);

        Node* load_from_resource(char const* path);
    };
}

