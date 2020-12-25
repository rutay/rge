#include "renderer.hpp"

#include <string>
#include <iostream>

#include <bx/sort.h>
#include <bx/math.h>

#include "resource/resource_provider.hpp"

// ------------------------------------------------------------------------------------------------
// Renderer
// ------------------------------------------------------------------------------------------------

using namespace rge;

rge::Renderer::Renderer()
{

    /*
    m_material_uniform = bgfx::createUniform("u_material", bgfx::UniformType::Enum::Vec4, 1);

    m_lights_uniform = bgfx::createUniform("u_lights", bgfx::UniformType::Enum::Mat3, 6); // 6xMat3
    m_lights_num_uniform = bgfx::createUniform("u_lights_num", bgfx::UniformType::Vec4, 1);

    m_camera_position_uniform = bgfx::createUniform("u_camera_position", bgfx::UniformType::Vec4, 1);*/
}

Renderer::~Renderer()
{}

void Renderer::set_camera(Camera const& camera)
{
    return;

    float cam_pos[4];
    cam_pos[0] = camera.m_position.x;
    cam_pos[1] = camera.m_position.y;
    cam_pos[2] = camera.m_position.z;

    bgfx::setUniform(m_camera_position_uniform, cam_pos, UINT16_MAX);
}

void Renderer::set_material(Material const* material)
{
    return;

    bgfx::setUniform(m_material_uniform, reinterpret_cast<float const*>(material), 1);
}

void Renderer::submit(bgfx::ViewId view_id, InstancedSubmitPacket const& packet)
{
    /*
    for (auto const& [mesh, nodes] : packet.m_mesh_users)
    {
        // If the instance data buffer hasn't been created yet for the curretn mesh' nodes, then we have to create it.
        if (!m_instance_data_buffer_by_mesh.contains(mesh))
        {
            size_t instances_num = nodes.size();
            size_t instance_stride = 16 * sizeof(float); // The only thing that needs to be instanced is the node's world transform.

            // Is it possible to instance a InstanceDataBuffer of the given size?
            if (instances_num == bgfx::getAvailInstanceDataBuffer(instances_num, instance_stride))
            {
                bgfx::InstanceDataBuffer instance_data_buffer;
                bgfx::allocInstanceDataBuffer(&instance_data_buffer, instances_num, instance_stride);

                uint8_t* data = instance_data_buffer.data;

                for (Node const* node : nodes)
                {
                    std::copy(node->m_world_transform, node->m_world_transform + 16, reinterpret_cast<float*>(data));

                    data += instance_stride;
                }

                m_instance_data_buffer_by_mesh.insert(std::pair(mesh, instance_data_buffer));
            }
            else
            {
                std::cerr << "Couldn't request InstanceDataBuffer, BGFX transient VBO memory is full..." << std::endl;
            }
        }

        if (m_instance_data_buffer_by_mesh.contains(mesh))
        {
            bgfx::setState(BGFX_STATE_DEFAULT & (~BGFX_STATE_CULL_CW));

            if (mesh->m_material)
                set_material(mesh->m_material);
            else
            {
                std::cerr << "No material? TODO use default material please." << std::endl;
                continue;
            }
            
            // instance data
            bgfx::InstanceDataBuffer& instance_data_buffer = m_instance_data_buffer_by_mesh.at(mesh);
            bgfx::setInstanceDataBuffer(&instance_data_buffer);

            // vbo
            for (int i = 0; i < mesh->m_vertex_buffers.size(); i++)
                bgfx::setVertexBuffer(i, mesh->m_vertex_buffers[i]);

            // ebo
            bgfx::setIndexBuffer(mesh->m_index_buffer);

            uint8_t flags = BGFX_DISCARD_NONE;

            flags |= BGFX_DISCARD_INSTANCE_DATA;
            flags |= BGFX_DISCARD_VERTEX_STREAMS;
            flags |= BGFX_DISCARD_INDEX_BUFFER;
            flags |= BGFX_DISCARD_STATE; // This is used to discard the uniforms.

            bgfx::submit(view_id, m_program, 0, flags);
        }
    }*/
}

void Renderer::linearize_scene_graph_r(Node* scene_graph, float parent_transform[16])
{
    Node& placed = m_render_meshes.emplace_back(*scene_graph);

    bx::mtxMul(placed.m_world_transform, parent_transform, placed.m_transform);

    if (placed.m_light)
    {
        m_render_lights.push_back(&placed);
    }

    for (auto child : scene_graph->m_children)
        linearize_scene_graph_r(child, placed.m_world_transform);
}

void Renderer::linearize_scene_graph(Node* scene_graph, float parent_transform[16])
{
    m_render_meshes.clear();
    m_render_lights.clear();

    linearize_scene_graph_r(scene_graph, parent_transform);

    bx::ComparisonFn compare_f = [](const void* a, const void* b) {
        auto node_a = reinterpret_cast<Node const*>(a);
        auto node_b = reinterpret_cast<Node const*>(b);

        return static_cast<int32_t>(node_a->m_mesh - node_b->m_mesh);
    };

    bx::quickSort(m_render_meshes.data(), m_render_meshes.size(), sizeof(Node), compare_f);
}

void Renderer::allocate_lights_volumes()
{
    for (Node* light_node : m_render_lights)
    {
        auto light = light_node->m_light;


    }
}

// ------------------------------------------------------------------------------------------------ Renderer_DrawMeshCommand

void Renderer_DrawMeshCommand::run(bgfx::ViewId view_id, Renderer* renderer)
{
    bgfx::setInstanceDataBuffer(m_instance_buffer, 0, 0);
    for (uint8_t stream_i = 0; stream_i < m_vertex_buffers.size(); stream_i++)
        bgfx::setVertexBuffer(stream_i, m_vertex_buffers[stream_i]);
    bgfx::setIndexBuffer(m_index_buffer);

    bgfx::submit(view_id, renderer->m_draw_mesh_program);
}

void Renderer_DrawMeshCommand::init_program()
{
    auto fs_shader = load_shader("assets/cli/shaders/draw_mesh_fs.bin");
    auto vs_shader = load_shader("assets/cli/shaders/simple_inst_vs.bin");
    
    m_program = bgfx::createProgram(vs_shader, fs_shader);
}

bgfx::VertexLayout bgfx_vertex_layout_from_attribute(bgfx::Attrib::Enum attrib_type, Accessor const* attrib_accessor)
{
    bgfx::VertexLayout vertex_layout;

    vertex_layout
        .begin()
        .add(attrib_type, attrib_accessor->m_components_num, bgfx::AttribType::Float, attrib_accessor->m_normalized)
        .end();

    return vertex_layout;
}

bgfx::Memory const* bgfx_memory_from_accessor(Accessor const* accessor)
{
    return bgfx::copy(accessor->m_data, accessor->m_size);
}

bgfx::VertexBufferHandle bgfx_vbo_from_attribute(bgfx::Attrib::Enum attrib_type, Accessor const* attrib_accessor)
{
    bgfx::VertexLayout vertex_layout = bgfx_vertex_layout_from_attribute(attrib_type, attrib_accessor);
    
    auto memory = bgfx_memory_from_accessor(attrib_accessor);
    auto vbo = bgfx::createVertexBuffer(memory, vertex_layout);
    return vbo;
}

void Renderer_DrawMeshCommand::create(Renderer_DrawMeshCommand& command, Mesh const* mesh, std::vector<Node const*> const& nodes)
{
    // Vertex buffers
    if (mesh->m_position_attribute)
    {
        auto vbo = bgfx_vbo_from_attribute(bgfx::Attrib::Enum::Position, mesh->m_position_attribute);
        command.m_vertex_buffers.push_back(vbo);
    }

    if (mesh->m_normal_attribute)
    {
        auto vbo = bgfx_vbo_from_attribute(bgfx::Attrib::Enum::Normal, mesh->m_normal_attribute);
        command.m_vertex_buffers.push_back(vbo);
    }

    if (mesh->m_color_0_attribute)
    {
        auto vbo = bgfx_vbo_from_attribute(bgfx::Attrib::Enum::Color0, mesh->m_color_0_attribute);
        command.m_vertex_buffers.push_back(vbo);
    }

    if (mesh->m_tangent_attribute)
    {
        auto vbo = bgfx_vbo_from_attribute(bgfx::Attrib::Enum::Tangent, mesh->m_tangent_attribute);
        command.m_vertex_buffers.push_back(vbo);
    }

    // Indices buffer
    if (mesh->m_indices)
    {
        auto memory = bgfx_memory_from_accessor(mesh->m_indices);
        auto ibo = bgfx::createIndexBuffer(memory);
        command.m_index_buffer = ibo;
    }

    // Instance buffer
    bgfx::Memory const* memory = bgfx::alloc(nodes.size() * 16 * sizeof(float));
    uint8_t* data = memory->data;
    for (Node const* node : nodes)
        bx::memCopy(data, node->m_world_transform, 16 * sizeof(float));

    bgfx::VertexLayout vertex_layout;
    vertex_layout
        .begin()
        .skip(16)
        .end();
    command.m_instance_buffer = bgfx::createVertexBuffer(memory, vertex_layout);
}

// ------------------------------------------------------------------------------------------------ Renderer_CreateShadowMapCommand

void Renderer_CreateShadowMapCommand::run(bgfx::ViewId view_id, Renderer* renderer)
{


}

void Renderer_CreateShadowMapCommand::init_program()
{
    // TODO
}

void Renderer_CreateShadowMapCommand::create(Renderer_CreateShadowMapCommand& command, Node const* light_node)
{
    // TODO
}

// ------------------------------------------------------------------------------------------------ Renderer_ApplyLightCommand

void Renderer_ApplyLightCommand::run(Node const* light_node, bgfx::ViewId view_id, Renderer* renderer)
{
    bgfx::TextureHandle shadow_map = renderer->m_shadow_map_by_light_node.at(light_node);

    // TODO

}

void Renderer_ApplyLightCommand::init_program()
{
    // TODO
}

void Renderer_ApplyLightCommand::create(Renderer_ApplyLightCommand& command, Node const* light_node)
{
    // TODO
}

// ------------------------------------------------------------------------------------------------ Renderer

void Renderer::render(bgfx::ViewId view_id, Node* scene_graph, PacketBuffer const& packet_buffer)
{
    std::unordered_map<Mesh*,  std::vector<Node const*>> nodes_by_mesh;
    std::vector<Node*> light_nodes;

    Node::traverse(scene_graph, [&](Node* node) {
        {
            auto mesh = node->m_mesh;
            auto [_, result] = nodes_by_mesh.insert({ mesh, std::vector<Node const*>() });
            if (!result)
                nodes_by_mesh.at(node->m_mesh).push_back(node);
        }

        if (node->m_light)
            light_nodes.push_back(node);
    });

    for (auto& [mesh, nodes] : nodes_by_mesh)
    {
        Renderer_DrawMeshCommand draw_mesh_cmd;
        Renderer_DrawMeshCommand::create(draw_mesh_cmd, mesh, nodes);
        draw_mesh_cmd.run(view_id, this);
    }


    /*
    TODO LIGHTING
       
    for (auto light_node : light_nodes)
    {
        Renderer_CreateShadowMapCommand create_shadow_map_cmd;
        Renderer_CreateShadowMapCommand::create(create_shadow_map_cmd, light_node);
        create_shadow_map_cmd.run(view_id, this);

        Renderer_ApplyLightCommand apply_light_cmd;
        Renderer_ApplyLightCommand::create(apply_light_cmd, light_node);
        apply_light_cmd.run(light_node, view_id, this);
    }*/
}
