
#include "renderer_gl.hpp"

#include "resources/resource_provider.hpp"
#include "material_serializer.hpp"

#include <cstring>

#include <iostream>
#include <filesystem>

#define RGE_MAX_MATERIAL_BYTE_SIZE 1024
using namespace rge;

#define GL_CHECK(_func_call) \
    { \
        _func_call; \
        GLenum _err = glGetError(); \
        if (_err != GL_NO_ERROR) {  \
            printf("%s (line %d) - %s 0x%x\n", __FILE__, __LINE__, #_func_call, _err); \
        } \
    }

GLuint load_shader_from_source(GLenum type, char const* src)
{
	GLuint shader = glCreateShader(type);

    GL_CHECK(glShaderSource(shader, 1, &src, nullptr));
    GL_CHECK(glCompileShader(shader));

	GLint success = 0;
	GL_CHECK(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
	if (success == GL_FALSE)
	{
		GLint log_length = 0;
		GL_CHECK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length));

		std::vector<GLchar> log(log_length);
		GL_CHECK(glGetShaderInfoLog(shader, log_length, &log_length, log.data()));

		printf("Shader %d compilation issue: %s\n", type, log.data());

        GL_CHECK(glDeleteShader(shader));
		exit(5);
	}

	return shader;
}

GLuint load_shader(GLenum type, resources::Shader shader)
{
    std::string src;
	ResourceProvider::read_to_string((Resource) shader, src);
	return load_shader_from_source(type, src.c_str());
}

GLuint load_material_shader(GLenum type, resources::Shader shader, Material const* material)
{
	return load_shader(type, (resources::Shader) ResourceManager::get_material_shader(shader, material));
}

//

GLenum parse_component_type(ComponentType component_type)
{
	GLenum result;
	switch (component_type)
	{
	case BYTE:           result = GL_BYTE;           break;
	case UNSIGNED_BYTE:  result = GL_UNSIGNED_BYTE;  break;
	case SHORT:          result = GL_SHORT;          break;
	case UNSIGNED_SHORT: result = GL_UNSIGNED_SHORT; break;
	case UNSIGNED_INT:   result = GL_UNSIGNED_INT;   break;
	case FLOAT:          result = GL_FLOAT;          break;
	}
	return result;
}

void buffer_data_from_accessor(GLenum buffer_type, Accessor const* accessor, GLenum buffer_usage)
{
	uint8_t* data =
		reinterpret_cast<uint8_t*>(accessor->m_buffer_view->m_buffer->m_data) +
			accessor->m_buffer_view->m_byte_offset +
			accessor->m_byte_offset;

	size_t size =
		accessor->m_buffer_view->m_byte_length;

	//printf("Alloc data on VBO: %d\n", (GLsizei) size);

	GL_CHECK(glBufferData(buffer_type, (GLsizei) size, data, buffer_usage));
}

GLuint RendererGL::get_or_create_mat_program(Material const* material)
{
    resources::Material mat_res = material->get_resource();

	if (!m_program_by_material_type.contains(mat_res))
	{
		GLuint program = glCreateProgram();

		GLuint v_shader = load_shader(GL_VERTEX_SHADER, resources::Shader::GLSL_VS_BasicInst);
		GLuint f_shader = load_material_shader(GL_FRAGMENT_SHADER, resources::Shader::GLSL_FS_IterLights, material);

        GL_CHECK(glAttachShader(program, v_shader));
        GL_CHECK(glAttachShader(program, f_shader));

        GL_CHECK(glLinkProgram(program));

        GLint is_linked = 0;
        GL_CHECK(glGetProgramiv(program, GL_LINK_STATUS, &is_linked));
        if (is_linked == GL_FALSE)
        {
            GLint max_len = 0;
            GL_CHECK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_len));

            std::vector<GLchar> info_log(max_len);
            GL_CHECK(glGetProgramInfoLog(program, max_len, &max_len, &info_log[0]));

            printf("Couldn't link the GLSL program for vertex_shader=%d fragment_shader=%d material=%d: %s\n",
                   resources::Shader::GLSL_VS_BasicInst,
                   resources::Shader::GLSL_FS_IterLights,
                   mat_res,
                   info_log.data()
                   );

            exit(3);
        }

        //printf("Program created for material type: 0x%x\n", mat_res);

		m_program_by_material_type[mat_res] = program;
	}

	return m_program_by_material_type[mat_res];
}

GLuint RendererGL::get_or_bake_mat_ubo(Material const* material)
{
	if (!m_ubo_by_material.contains(material))
	{
		size_t size;
		uint8_t data[RGE_MAX_MATERIAL_BYTE_SIZE];

		MaterialSerializerManager::serialize(material, data, size);

		GLuint uniform_buffer;
        GL_CHECK(glGenBuffers(1, &uniform_buffer));
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer));
        GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW));

        //printf("Material baked: %p\n", material);

		m_ubo_by_material.insert({ material, uniform_buffer });
	}

	return m_ubo_by_material[material];
}

RendererGL::BakedAttribute bake_attrib(AttribType::Enum attrib_type, Accessor const* accessor)
{
    RendererGL::BakedAttribute res;

    GL_CHECK(glGenBuffers(1, &res.m_vbo));
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, res.m_vbo));
    buffer_data_from_accessor(GL_ARRAY_BUFFER, accessor, GL_STATIC_DRAW);

    GL_CHECK(glGenVertexArrays(1, &res.m_vao));
    GL_CHECK(glBindVertexArray(res.m_vao));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, res.m_vbo));

    GL_CHECK(glEnableVertexAttribArray(attrib_type));
    GL_CHECK(glVertexAttribPointer(
            attrib_type,
            accessor->m_num_components,
            parse_component_type(accessor->m_component_type),
            false,
            accessor->get_value_byte_size(),
            nullptr
    ));

    return res;
}

RendererGL::BakedGeometry& RendererGL::get_or_bake_geometry(Geometry const* geometry)
{
	if (!m_baked_geometries.contains(geometry))
	{
		RendererGL::BakedGeometry baked_geom;

		for (int attrib_type = 0; attrib_type < AttribType::Count; attrib_type++)
		{
			if (geometry->m_attributes[attrib_type])
			{
			    BakedAttribute baked_attrib = bake_attrib(static_cast<AttribType::Enum>(attrib_type), geometry->m_attributes[attrib_type]);
                baked_geom.m_baked_attribs.push_back(baked_attrib);
			}
		}

		if (geometry->m_indices)
		{
			Accessor const* accessor = geometry->m_indices;

            baked_geom.m_ebo_elem_type = parse_component_type(accessor->m_component_type);
            baked_geom.m_ebo_elem_count = accessor->m_count;

            GL_CHECK(glGenBuffers(1, &baked_geom.m_ebo));
            GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baked_geom.m_ebo));
			buffer_data_from_accessor(GL_ELEMENT_ARRAY_BUFFER, accessor, GL_STATIC_DRAW);
		}

		//printf("Geometry baked: %p\n", geometry);

		m_baked_geometries.emplace(geometry, baked_geom);
	}

	return m_baked_geometries[geometry];
}

void RendererGL::bake_scene_graph(Node const* scene_graph)
{
    if (m_baked_instances_by_root_node.contains(scene_graph))
    {
        return;
    }

    std::unordered_map<Mesh const*, std::vector<Node const*>> nodes_by_mesh;
    std::vector<Node const*> nodes_with_lights;

    scene_graph->traverse_const([&](Node const* node) {
        for (Mesh const* mesh : node->m_meshes)
        {
            nodes_by_mesh.emplace(mesh, std::vector<Node const*>());
            nodes_by_mesh[mesh].push_back(node);
        }

        if (node->m_light)
            nodes_with_lights.push_back(node);
    });

    std::vector<std::tuple<Mesh const*, GLuint>> mesh_instances;
    for (auto& [mesh, nodes] : nodes_by_mesh)
    {
        // material
        if (mesh->m_material)
        {
            get_or_create_mat_program(mesh->m_material);
            get_or_bake_mat_ubo(mesh->m_material);
        }

        // geometry
        RendererGL::BakedGeometry& baked_geom = get_or_bake_geometry(mesh->m_geometry);

        size_t size = 16 * nodes.size() * sizeof(float);
        float* data = (float*) malloc(size);
        for (int i = 0; i < nodes.size(); i++) {
            std::memcpy(
                    &data[i * 16],
                    nodes[i]->m_world_transform,
                    16 * sizeof(float)
            );
        }

        GLuint instance_data_buffer;
        GL_CHECK(glGenBuffers(1, &instance_data_buffer));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, instance_data_buffer));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) size, data, GL_STATIC_DRAW));

        free(data);

        mesh_instances.emplace_back(mesh, instance_data_buffer);
        baked_geom.m_ibo_inst_count = nodes.size();
    }

    m_baked_instances_by_root_node.emplace(scene_graph, mesh_instances);
}


void RendererGL::render(Node const* scene_graph, Camera const& camera)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);

    bake_scene_graph(scene_graph);

	std::vector<std::tuple<Mesh const*, GLuint>>& mesh_instances = m_baked_instances_by_root_node[scene_graph];

	for (auto& [mesh, ibo] : mesh_instances)
	{
		GLuint program = get_or_create_mat_program(mesh->m_material);
        GL_CHECK(glUseProgram(program));

        GLuint material_ubo = get_or_bake_mat_ubo(mesh->m_material);
        //GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, RGE_BUFFER_IDX_MATERIAL, material_ubo));

		auto& baked_geom = get_or_bake_geometry(mesh->m_geometry);

		for (RendererGL::BakedAttribute& baked_attrib : baked_geom.m_baked_attribs) // Attributes
		{
			GL_CHECK(glBindVertexArray(baked_attrib.m_vao));
			//glBindBuffer(GL_ARRAY_BUFFER, baked_attrib.m_vertex_buffer);
		}

        glDisableVertexAttribArray(AttribType::Enum::NORMAL);
		glVertexAttrib3f(AttribType::Enum::NORMAL, 0.0f, 1.0f, 0.0f);

        glDisableVertexAttribArray(AttribType::Enum::COLOR_0);
        glVertexAttrib3f(AttribType::Enum::COLOR_0, 0.0f, 1.0f, 0.0f);

		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baked_geom.m_ebo));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, ibo));

        GL_CHECK(glDrawElementsInstanced(
                GL_TRIANGLES,
                baked_geom.m_ebo_elem_count,
                baked_geom.m_ebo_elem_type,
                nullptr,
                baked_geom.m_ibo_inst_count
                ));
	}
}

Renderer* Renderer::create()
{
	return new RendererGL();
}
