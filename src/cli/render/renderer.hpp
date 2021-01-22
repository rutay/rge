#pragma once

#include <set>

#include <bgfx/bgfx.h>

#include "scene/scene.hpp"
#include "camera.hpp"

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
        static bgfx::ProgramHandle s_program;
        static bgfx::UniformHandle s_material_data0_uniform;
        static bgfx::UniformHandle s_material_data1_uniform;

        bgfx::VertexBufferHandle m_instance_buffer;

        std::vector<bgfx::VertexBufferHandle> m_vertex_buffers;
        bgfx::IndexBufferHandle m_index_buffer;

        void run(bgfx::ViewId view_id, Camera const& camera);
        void destroy();

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

    struct Renderer_SceneGraphCache
    {
        std::unordered_map<Mesh*, std::vector<Node const*>> m_nodes_by_mesh;
        std::vector<Renderer_DrawMeshCommand> m_draw_mesh_commands;

        void remap_by_mesh(Node const* scene_graph);
        void rebuild_draw_mesh_commands();
        void rebuild(Node const* scene_graph);

        void render(bgfx::ViewId view_id, Camera const& camera);
    };

    struct Renderer
    {
        std::unordered_map<Node*, Renderer_SceneGraphCache> m_cache_by_scene_graph;

        Renderer();
        ~Renderer();

        void render(bgfx::ViewId view_id, Node* scene_graph, Camera const& camera);
    };
}
