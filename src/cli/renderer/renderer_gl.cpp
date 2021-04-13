
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

		GLuint ubo;
        GL_CHECK(glGenBuffers(1, &ubo));
        GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ubo));
        GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW));

        //printf("Material baked: %p\n", material);

		m_ubo_by_material.emplace(material, ubo);
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
    std::vector<Light> lights;

    scene_graph->traverse_const([&](Node const* node) {
        for (Mesh const* mesh : node->m_meshes)
        {
            nodes_by_mesh.emplace(mesh, std::vector<Node const*>());
            nodes_by_mesh[mesh].push_back(node);
        }

        if (node->m_light) {
			lights.push_back(*node->m_light);
		}
    });

    // Lights
    GLuint lights_ubo;
    glGenBuffers(1, &lights_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    glBufferData(GL_UNIFORM_BUFFER, lights.size() * sizeof(Light), lights.data(), GL_STATIC_DRAW);
	m_lights_ubo_by_scene_graph.emplace(scene_graph, LightsUBO{
		.m_idx = lights_ubo,
		.m_lights_count = lights.size()
	});

	// Meshes
    std::vector<std::tuple<Mesh const*, GLuint>> mesh_instances;
    for (auto& [mesh, nodes] : nodes_by_mesh)
    {
        // Material
        if (mesh->m_material)
        {
            get_or_create_mat_program(mesh->m_material);
            get_or_bake_mat_ubo(mesh->m_material);
        }

        // Geometry
        RendererGL::BakedGeometry& baked_geom = get_or_bake_geometry(mesh->m_geometry);

        // Instances
        std::vector<float> inst_data(16 * nodes.size());
        float* inst_data_ptr = inst_data.data();

        for (int i = 0; i < nodes.size(); i++) {
        	float const* node_transf = nodes[i]->m_world_transform;
        	std::memcpy(
				inst_data_ptr,
        		node_transf,
        		16 * sizeof(float)
        	);
			inst_data_ptr += 16;
        }

        GLuint ibo;
        GL_CHECK(glGenBuffers(1, &ibo));
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, ibo));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, inst_data.size() * sizeof(float), inst_data.data(), GL_STATIC_DRAW));

        mesh_instances.emplace_back(mesh, ibo);
        baked_geom.m_ibo_inst_count = nodes.size();
    }

    m_baked_instances_by_root_node.emplace(scene_graph, mesh_instances);
}

void bind_per_instance_attribs()
{
	// todo create a vao for that maybe?

	static const int c_i_model_attr = 8;

	GLuint attrib_loc;

	// i_model_0
	attrib_loc = c_i_model_attr;
	GL_CHECK(glEnableVertexAttribArray(attrib_loc));
	GL_CHECK(glVertexAttribPointer(attrib_loc, 4, GL_FLOAT, false, sizeof(GLfloat) * 16, (void*) 0));
	glVertexAttribDivisor(attrib_loc, 1);

	// i_model_1
	attrib_loc = c_i_model_attr + 1;
	GL_CHECK(glEnableVertexAttribArray(attrib_loc));
	GL_CHECK(glVertexAttribPointer(attrib_loc, 4, GL_FLOAT, false, sizeof(GLfloat) * 16, (void*) (sizeof(GLfloat) * 4)));
	glVertexAttribDivisor(attrib_loc, 1);

	// i_model_2
	attrib_loc = c_i_model_attr + 2;
	GL_CHECK(glEnableVertexAttribArray(attrib_loc));
	GL_CHECK(glVertexAttribPointer(attrib_loc, 4, GL_FLOAT, false, sizeof(GLfloat) * 16, (void*) (sizeof(GLfloat) * (4 + 4))));
	glVertexAttribDivisor(attrib_loc, 1);

	// i_model_3
	attrib_loc = c_i_model_attr + 3;
	GL_CHECK(glEnableVertexAttribArray(attrib_loc));
	GL_CHECK(glVertexAttribPointer(attrib_loc, 4, GL_FLOAT, false, sizeof(GLfloat) * 16, (void*) (sizeof(GLfloat) * (4 + 4 + 4))));
	glVertexAttribDivisor(attrib_loc, 1);
}

void RendererGL::render(Node const* scene_graph, Camera const& camera)
{
	glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);

    bake_scene_graph(scene_graph);

	std::vector<std::tuple<Mesh const*, GLuint>>& mesh_instances = m_baked_instances_by_root_node[scene_graph];

	for (auto& [mesh, ibo] : mesh_instances)
	{
		GLuint program = get_or_create_mat_program(mesh->m_material);
		GL_CHECK(glUseProgram(program));

		float camera_view[16], camera_proj[16];
		camera.view_matrix(camera_view);
		camera.projection_matrix(camera_proj);

		GLint u_camera_view = glGetUniformLocation(program, "u_camera_view");
		GLint u_camera_projection = glGetUniformLocation(program, "u_camera_projection");
		GL_CHECK(glUniformMatrix4fv(u_camera_view, 1, GL_FALSE, camera_view));
		GL_CHECK(glUniformMatrix4fv(u_camera_projection, 1, GL_FALSE, camera_proj));

		// Material
        GLuint material_ubo = get_or_bake_mat_ubo(mesh->m_material);
		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "rge_b_material"), 0);
		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, material_ubo));
        GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 0, material_ubo));

        // Lights
		LightsUBO lights_ubo = m_lights_ubo_by_scene_graph[scene_graph];

		GLint u_lights_count = glGetUniformLocation(program, "u_lights_count");
		GL_CHECK(glUniform1i(u_lights_count, lights_ubo.m_lights_count));

		glUniformBlockBinding(program, glGetUniformBlockIndex(program, "rge_b_lights"), 1);
		GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo.m_idx));
		GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, 1, lights_ubo.m_idx));

		// Geometry
		auto& baked_geom = get_or_bake_geometry(mesh->m_geometry);

		for (RendererGL::BakedAttribute& baked_attrib : baked_geom.m_baked_attribs) // Attributes
		{
			GL_CHECK(glBindVertexArray(baked_attrib.m_vao));
			//glBindBuffer(GL_ARRAY_BUFFER, baked_attrib.m_vertex_buffer);
		}

		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baked_geom.m_ebo));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, ibo));
		bind_per_instance_attribs();

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
