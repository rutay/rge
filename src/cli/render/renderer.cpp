#include "renderer.hpp"

#include <string>
#include <iostream>

#include <bx/sort.h>
#include <bx/math.h>

#include "resource/resource_provider.hpp"

#include "rge.hpp"

// ------------------------------------------------------------------------------------------------
// Renderer
// ------------------------------------------------------------------------------------------------

using namespace rge;

bgfx::Memory const* bgfx_copy_memory_from_buffer(std::vector<uint8_t> const& buffer)
{
    return bgfx::copy(buffer.data(), buffer.size());
}

bgfx::Memory const* bgfx_copy_memory_from_accessor_buffer(rge::AccessorBuffer const* accessor)
{
    return bgfx_copy_memory_from_buffer(accessor->m_data);
}

bool bgfx_load_shader(bgfx::ShaderHandle& result, char const* shader_filename)
{
    std::vector<uint8_t> buffer;
    size_t size = ResourceProvider::request(shader_filename, buffer);
    if (size)
    {
        bgfx::Memory const* memory = bgfx_copy_memory_from_buffer(buffer);
        result = bgfx::createShader(memory);
        return true;
    }
    return false;
}

Renderer::Renderer()
{
    Renderer_DrawMeshCommand::init_program();
}

Renderer::~Renderer()
{}

// ------------------------------------------------------------------------------------------------ Renderer_DrawMeshCommand

bgfx::ProgramHandle Renderer_DrawMeshCommand::s_program = BGFX_INVALID_HANDLE;
bgfx::UniformHandle Renderer_DrawMeshCommand::s_material_data0_uniform = BGFX_INVALID_HANDLE;
bgfx::UniformHandle Renderer_DrawMeshCommand::s_material_data1_uniform = BGFX_INVALID_HANDLE;

void Renderer_DrawMeshCommand::run(bgfx::ViewId view_id, Camera const& camera)
{
    for (int stream = 0; stream < m_vertex_buffers.size(); stream++)
        bgfx::setVertexBuffer(stream, m_vertex_buffers[stream]);
    bgfx::setIndexBuffer(m_index_buffer);

	bgfx::setInstanceDataBuffer(m_instance_buffer, 0, m_instances_count);

    float camera_view[16], camera_proj[16];
    camera.view(camera_view);
    camera.projection(camera_proj);
    bgfx::setViewTransform(view_id, camera_view, camera_proj);

	bgfx::setState(BGFX_STATE_DEFAULT);

	bgfx::submit(view_id, Renderer_DrawMeshCommand::s_program);

    float material_data[4] = {0, 0, 0, 0}; // TODO
    bgfx::setUniform(s_material_data0_uniform, material_data);
    bgfx::setUniform(s_material_data1_uniform, material_data);
}

void Renderer_DrawMeshCommand::destroy()
{
    bgfx::destroy(m_instance_buffer);
    bgfx::destroy(m_index_buffer);
    for (bgfx::VertexBufferHandle vbo : m_vertex_buffers)
        bgfx::destroy(vbo);
}

bgfx::Attrib::Enum bgfx_parse_attrib_type(rge::AttribType attrib_type)
{
    switch (attrib_type)
    {
    case POSITION:   return bgfx::Attrib::Enum::Position;
    case NORMAL:     return bgfx::Attrib::Enum::Normal;
    case TEXCOORD_0: return bgfx::Attrib::Enum::TexCoord0;
    case TEXCOORD_1: return bgfx::Attrib::Enum::TexCoord1;
    case COLOR_0:    return bgfx::Attrib::Enum::Color0;
        //case JOINTS_0: return bgfx::Attrib::Enum::
    case WEIGHTS_0:  return bgfx::Attrib::Enum::Weight;
    default:         return bgfx::Attrib::Enum::Count;
    }
}

bgfx::AttribType::Enum bgfx_parse_component_type(rge::ComponentType comp_type)
{
    switch (comp_type)
    {
        case rge::ComponentType::FLOAT:         return bgfx::AttribType::Enum::Float;
        case rge::ComponentType::SHORT:         return bgfx::AttribType::Enum::Int16;
        case rge::ComponentType::UNSIGNED_BYTE: return bgfx::AttribType::Enum::Uint8;
        default:                                return bgfx::AttribType::Enum::Count;
    }
}

bool bgfx_create_vertex_layout(bgfx::VertexLayout& result, rge::AttribType attrib_type, AccessorBuffer const* accessor)
{
    bgfx::Attrib::Enum bgfx_attrib_type = bgfx_parse_attrib_type(attrib_type);
    if (bgfx_attrib_type == bgfx::Attrib::Enum::Count)
    {
        printf("No BGFX matching for the attrib type: %d\n", attrib_type);
        return false;
    }

    bgfx::AttribType::Enum bgfx_comp_type = bgfx_parse_component_type(accessor->m_component_type);
    if (bgfx_comp_type == bgfx::AttribType::Enum::Count)
    {
        printf("No BGFX matching for the component type: %d\n", accessor->m_component_type);
        return false;
    }

    result
        .begin()
        .add(bgfx_attrib_type, accessor->m_num_components, bgfx_comp_type, accessor->m_normalized)
        .skip(accessor->m_stride - accessor->get_data_size())
        .end();

    return true;
}

bool bgfx_allocate_vbo_from_attrib_accessor(bgfx::VertexBufferHandle& result, rge::AttribType attrib_type, AccessorBuffer const* accessor)
{
    bgfx::VertexLayout vertex_layout;
    if (!bgfx_create_vertex_layout(vertex_layout, attrib_type, accessor))
    {
        printf("Vertex layout failed to initialize\n");
        return false;
    }

    bgfx::Memory const* memory = bgfx_copy_memory_from_accessor_buffer(accessor);
    result = bgfx::createVertexBuffer(memory, vertex_layout);
    return true;
}

bool bgfx_allocate_ibo_from_accessor(bgfx::IndexBufferHandle& result, AccessorBuffer const* accessor)
{
    bgfx::Memory const* memory = bgfx_copy_memory_from_accessor_buffer(accessor);

    uint16_t indices_type;
    if    (accessor->m_component_type == rge::ComponentType::UNSIGNED_SHORT) indices_type = BGFX_BUFFER_NONE;
    else if (accessor->m_component_type == rge::ComponentType::UNSIGNED_INT) indices_type = BGFX_BUFFER_INDEX32;
    else {
    	printf("Couldn't match the requested IBO component type: %d.", accessor->m_component_type);
    	return false;
    }
    result = bgfx::createIndexBuffer(memory, indices_type);
    return true;
}

bool bgfx_allocate_idb_from_nodes(bgfx::VertexBufferHandle& result, std::vector<rge::Node const*> const& nodes)
{
    bgfx::Memory const* memory = bgfx::alloc(nodes.size() * 16 * sizeof(float));
    uint8_t* data = memory->data;
    for (rge::Node const* node : nodes)
        bx::memCopy(data, node->m_world_transform, 16 * sizeof(float));

    bgfx::VertexLayout vertex_layout;
    vertex_layout
        .begin()
        .skip(16 * sizeof(float))
        .end();
    result = bgfx::createVertexBuffer(memory, vertex_layout);

    return true;
}

void Renderer_DrawMeshCommand::init_program()
{
    if (!bgfx::isValid(s_program))
    {
        printf("Initializing draw_mesh program\n");

        bgfx::ShaderHandle v_shader{}, f_shader{};
        bgfx_load_shader(v_shader, RGE_asset("assets/cli/shaders/simple_inst_vs.bin"));
        bgfx_load_shader(f_shader, RGE_asset("assets/cli/shaders/draw_mesh_fs.bin"));

        s_program = bgfx::createProgram(v_shader, f_shader);

        s_material_data0_uniform = bgfx::createUniform("u_material_data0", bgfx::UniformType::Vec4);
        s_material_data1_uniform = bgfx::createUniform("u_material_data1", bgfx::UniformType::Vec4);
    }
}

void Renderer_DrawMeshCommand::create(Renderer_DrawMeshCommand& command, Mesh const* mesh, std::vector<Node const*> const& nodes)
{
    for (int attrib_type = 0; attrib_type < AttribType::Count; attrib_type++)
    {
        AccessorBuffer* attrib_accessor = mesh->m_attributes[attrib_type];
        if (attrib_accessor)
        {
            bgfx::VertexBufferHandle vbo{};
            if (!bgfx_allocate_vbo_from_attrib_accessor(vbo, static_cast<rge::AttribType>(attrib_type), attrib_accessor))
            {
                printf("Failed to initialize VBO for attrib: %d\n", attrib_type);
                continue;
            }
            command.m_vertex_buffers.push_back(vbo);

			printf("Allocated data for attrib %d\n", attrib_type);
        }
    }

    AccessorBuffer* indices_accessor = mesh->m_indices;
    if (indices_accessor)
    {
        bgfx::IndexBufferHandle ibo{};
        if (!bgfx_allocate_ibo_from_accessor(ibo, indices_accessor))
        {
            printf("Failed to initialize IBO\n");
            return;
        }
        command.m_index_buffer = ibo;
    }
    else
    {
        printf("Mesh without IBO? Weird...\n");
    }

    bgfx::VertexBufferHandle idb{};
    bgfx_allocate_idb_from_nodes(idb, nodes);
    command.m_instance_buffer = idb;
    command.m_instances_count = nodes.size();
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
    //bgfx::TextureHandle shadow_map = renderer->m_shadow_map_by_light_node.at(light_node);
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

// ------------------------------------------------------------------------------------------------ Renderer_SceneGraphCache

void Renderer_SceneGraphCache::remap_by_mesh(Node const* scene_graph)
{
    m_nodes_by_mesh.clear();

    scene_graph->traverse_const([&](Node const* node) {
        Mesh* mesh = node->m_mesh;

        m_nodes_by_mesh.insert({ mesh, std::vector<Node const*>() });
        m_nodes_by_mesh.at(node->m_mesh).push_back(node);
    });
}

void Renderer_SceneGraphCache::rebuild_draw_mesh_commands()
{
    m_draw_mesh_commands.clear();

    for (auto& [mesh, nodes] : m_nodes_by_mesh)
    {
        if (mesh)
        {
            Renderer_DrawMeshCommand draw_mesh_cmd;
            Renderer_DrawMeshCommand::create(draw_mesh_cmd, mesh, nodes);
            m_draw_mesh_commands.push_back(draw_mesh_cmd);

			printf("Mesh with %llu nodes allocated\n", nodes.size());
        }
    }
}

void Renderer_SceneGraphCache::rebuild(Node const* scene_graph)
{
    printf("Rebuilding scene_graph cache\n");

    remap_by_mesh(scene_graph);
    rebuild_draw_mesh_commands();
}

void Renderer_SceneGraphCache::render(bgfx::ViewId view_id, Camera const& camera)
{
    for (Renderer_DrawMeshCommand& draw_mesh_cmd : m_draw_mesh_commands)
    {
        draw_mesh_cmd.run(view_id, camera);
    }
}

// ------------------------------------------------------------------------------------------------ Renderer

void Renderer::render(bgfx::ViewId view_id, Node* scene_graph, Camera const& camera)
{
    if (!m_cache_by_scene_graph.contains(scene_graph))
    {
        Renderer_SceneGraphCache cache;
        cache.rebuild(scene_graph);
        m_cache_by_scene_graph.insert({ scene_graph, cache });
    }

    m_cache_by_scene_graph.at(scene_graph).render(view_id, camera);
}
