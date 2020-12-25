#pragma once

#include <set>

#include <bgfx/bgfx.h>

#include "scene/scene.hpp"
#include "camera.hpp"

#include "node.hpp"

namespace rge
{
    struct Renderer;

    template<typename L>
    struct BakedLight
    {
        bgfx::TextureHandle m_shadow_map;
        int m_volume_instance_idx;

        void rebuild_shadow_map();
        void update_volume_transform();
    };

    struct Renderer_DrawMeshCommand
    {
        static bgfx::ProgramHandle m_program;

        bgfx::VertexBufferHandle m_instance_buffer;

        std::vector<bgfx::VertexBufferHandle> m_vertex_buffers;
        bgfx::IndexBufferHandle m_index_buffer;

        void run(bgfx::ViewId view_id, Renderer* renderer);

        static void init_program();
        static void create(Renderer_DrawMeshCommand& command, Mesh const* mesh, std::vector<Node const*> const& node);
    };

    struct Renderer_CreateShadowMapCommand
    {
        static bgfx::ProgramHandle m_program;

        bgfx::TextureHandle m_shadow_map_texture;
        float m_camera_matrix[16];

        void run(bgfx::ViewId view_id, Renderer* renderer);

        static void init_program();
        static void create(Renderer_CreateShadowMapCommand& command, Node const* light_node);
    };

    struct Renderer_ApplyLightCommand
    {
        static bgfx::ProgramHandle m_program;

        bgfx::VertexBufferHandle m_vertex_buffer;

        void run(Node const* light_node, bgfx::ViewId view_id, Renderer* renderer);

        static void init_program();
        static void create(Renderer_ApplyLightCommand& command, Node const* light_node);
    };

    struct Renderer
    {
        bgfx::ProgramHandle m_draw_mesh_program;

        bgfx::UniformHandle m_camera_position_uniform;

        bgfx::UniformHandle m_material_uniform;

        bgfx::UniformHandle m_lights_uniform;
        bgfx::UniformHandle m_lights_num_uniform;

        std::unordered_map<Node const*, bgfx::TextureHandle> m_shadow_map_by_light_node;

        std::unordered_map<Mesh const*, bgfx::InstanceDataBuffer> m_instance_data_buffer_by_mesh;

        bgfx::TextureHandle m_positions_tex;
        bgfx::TextureHandle m_normals_tex;
        bgfx::TextureHandle m_colors_tex;

        std::vector<Node>  m_render_meshes;
        std::vector<Node*> m_render_lights;

        std::unordered_map<Mesh*, std::set<Node*>> m_nodes_by_mesh;
        //std::unordered_map<Mesh*, BakedMesh> m_baked_meshes_by_mesh;

        Renderer();
        ~Renderer();

        void set_camera(Camera const& camera);
        void set_lights(std::vector<Light> const& lights);
        void set_material(Material const* material);

        void traverse(Node* scene_graph);

        void linearize_scene_graph_r(Node* scene_graph, float parent_transform[16]);
        void linearize_scene_graph(Node* scene_graph, float parent_transform[16]);

        // Lighting

        void allocate_instances_data();
        void allocate_lights_volumes();


        void rebuild_instances(bgfx::InstanceDataBuffer& idb, std::set<Node*> nodes);

        void render(bgfx::ViewId view_id, Node* scene_graph, PacketBuffer const& packet_buffer);
    };

}
