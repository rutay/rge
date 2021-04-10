
#include "renderer_gl.hpp"

#include "resources/resource_provider.hpp"
#include "material_serializer.hpp"

#include <cstring>

#include <iostream>
#include <filesystem>

#define RGE_BUFFER_IDX_LIGHTS   1
#define RGE_BUFFER_IDX_MATERIAL 2

#define RGE_MAX_MATERIAL_BYTE_SIZE 1024

using namespace rge;

GLuint load_shader_from_source(GLenum type, char const* src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint log_length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

		std::vector<GLchar> log(log_length);
		glGetShaderInfoLog(shader, log_length, &log_length, log.data());

		glDeleteShader(shader);
		return NULL;
	}

	return shader;
}

GLuint load_shader(GLenum type, resources::Shader shader)
{
	std::vector<char> shader_src;
	ResourceProvider::read(shader, shader_src);
	return load_shader_from_source(type, reinterpret_cast<char const*>(shader_src.data()));
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

	glBufferData(buffer_type, size, data, buffer_usage);
}

RendererGL::BakedAttribute bake_attrib(AttribType::Enum attrib_type, Accessor const* accessor)
{
	RendererGL::BakedAttribute result;

	glGenVertexArrays(1, &result.m_vertex_array);
	glBindVertexArray(result.m_vertex_array);

	glEnableVertexAttribArray(attrib_type);
	glVertexAttribPointer(
		attrib_type,
		accessor->m_num_components,
		parse_component_type(accessor->m_component_type),
		false,
		accessor->get_value_byte_size(),
		NULL
	);

	glGenBuffers(1, &result.m_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, result.m_vertex_buffer);
	buffer_data_from_accessor(GL_ARRAY_BUFFER, accessor, GL_STATIC_DRAW);

	glBindVertexArray(NULL);

	return result;
}

GLuint RendererGL::get_or_create_program(Material const* material)
{
	if (!m_program_by_material_type[material->get_resource()])
	{
		GLuint program = glCreateProgram();

		GLuint v_shader = load_shader(GL_VERTEX_SHADER, resources::Shader::GLSL_VS_BasicInst);
		GLuint f_shader = load_material_shader(GL_FRAGMENT_SHADER, resources::Shader::GLSL_FS_IterLights, material);

		glAttachShader(program, v_shader);
		glAttachShader(program, f_shader);

		glLinkProgram(program);

		m_program_by_material_type[material->get_resource()] = program;
	}

	return m_program_by_material_type[material->get_resource()];
}

GLuint RendererGL::get_or_bake_material_ubo(Material const* material)
{
	if (!m_ubo_by_material.contains(material))
	{
		size_t size;
		uint8_t data[RGE_MAX_MATERIAL_BYTE_SIZE];

		MaterialSerializerManager::serialize(material, data, size);

		GLuint uniform_buffer;
		glGenBuffers(1, &uniform_buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer);
		glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);

		m_ubo_by_material.insert({ material, uniform_buffer });
	}

	return m_ubo_by_material[material];
}

RendererGL::BakedGeometry& RendererGL::get_or_bake_geometry(Geometry const* geometry)
{
	if (!m_baked_geometries.contains(geometry))
	{
		RendererGL::BakedGeometry baked_geometry;

		for (int attrib_type = 0; attrib_type < AttribType::Count; attrib_type++) {
			if (geometry->m_attributes[attrib_type]) {
				baked_geometry.m_baked_attributes[attrib_type] =
					bake_attrib(
						static_cast<AttribType::Enum>(attrib_type),
						geometry->m_attributes[attrib_type]
					);
			}
		}

		if (geometry->m_indices) {
			Accessor const* accessor = geometry->m_indices;

			baked_geometry.m_indices_type = parse_component_type(accessor->m_component_type);

			glGenBuffers(1, &baked_geometry.m_indices_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baked_geometry.m_indices_buffer);
			buffer_data_from_accessor(GL_ARRAY_BUFFER, accessor, GL_STATIC_DRAW);
		}

		m_baked_geometries.insert({ geometry, baked_geometry });
	}

	return m_baked_geometries[geometry];
}

void RendererGL::render(Node const* root_node, Camera const& camera)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(m_clear_color.r, m_clear_color.g, m_clear_color.b, m_clear_color.a);

	if (!m_baked_instances_by_root_node.contains(root_node))
	{
		std::unordered_map<Mesh const*, std::vector<Node const*>> nodes_by_mesh;
		std::vector<Node const*> nodes_with_lights;

		root_node->traverse_const([&](Node const* node) {
			for (Mesh const* mesh : node->m_meshes)
			{
				nodes_by_mesh.insert({ mesh, std::vector<Node const*>() });
				nodes_by_mesh[mesh].push_back(node);
			}

			if (node->m_light) {
				nodes_with_lights.push_back(node);
			}
		});

		//

		std::vector<std::tuple<Mesh const*, GLuint>> mesh_instances;

		for (auto& [mesh, nodes] : nodes_by_mesh)
		{
			get_or_bake_geometry(mesh->m_geometry);

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
			glGenBuffers(1, &instance_data_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, instance_data_buffer);
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

			free(data);

			mesh_instances.emplace_back(mesh, instance_data_buffer);
		}

		m_baked_instances_by_root_node.emplace(root_node, mesh_instances);

		//

		struct UBOLight
		{
			Vec3 position;
			Vec3 color;
			float intensity;

			GLint type;
			Vec4 data[2];
		};

		UBOLight* data = (UBOLight*) malloc(nodes_with_lights.size() * sizeof(UBOLight));
		for (int i = 0; i < nodes_with_lights.size(); i++)
		{
			Node const* node = nodes_with_lights[i];
			Light const* light = node->m_light;

			data[i].position = node->m_position;
			data[i].color = light->m_color;
			data[i].intensity = light->m_intensity;
			data[i].type = (GLuint) light->m_type;

			if (light->m_type == LightType::POINT)
			{
				PointLight const* point_light = static_cast<PointLight const*>(light);
				data[i].data->x = point_light->m_radius;
			}
			else if (light->m_type == LightType::DIRECTIONAL)
			{
				DirectionalLight const* directional_light = static_cast<DirectionalLight const*>(light);
				data[i].data[0] = directional_light->m_direction;
			}
			else if (light->m_type == LightType::SPOT)
			{
				SpotLight const* spot_light = static_cast<SpotLight const*>(light);
				data[i].data[0] = Vec4(
					spot_light->m_direction,
					spot_light->m_angle
				);
			}
			else
			{
				data[i].type = -1; // Unsupported light type
				continue;
			}
		}

		GLuint lights_ubo;
		glGenBuffers(1, &lights_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
		glBufferData(GL_UNIFORM_BUFFER, nodes_with_lights.size() * sizeof(UBOLight), data, GL_STATIC_DRAW);

		free(data);

		m_baked_lights_by_root_node.insert({ root_node, lights_ubo });
	}

	auto& mesh_instances = m_baked_instances_by_root_node[root_node];
	for (auto& [mesh, instance_data_buffer] : mesh_instances)
	{
		GLuint program = get_or_create_program(mesh->m_material);
		GLuint material_ubo = get_or_bake_material_ubo(mesh->m_material);
		glUseProgram(program);

		glBindBufferBase(GL_UNIFORM_BUFFER, RGE_BUFFER_IDX_LIGHTS, m_baked_lights_by_root_node[root_node]); // Lights
		glBindBufferBase(GL_UNIFORM_BUFFER, RGE_BUFFER_IDX_MATERIAL, material_ubo); // Material

		auto& baked_geometry = get_or_bake_geometry(mesh->m_geometry);

		for (RendererGL::BakedAttribute& baked_attrib : baked_geometry.m_baked_attributes) // Attributes
		{
			glBindVertexArray(baked_attrib.m_vertex_array);
			//glBindBuffer(GL_ARRAY_BUFFER, baked_attrib.m_vertex_buffer);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, baked_geometry.m_indices_buffer); // Indices

		glBindBuffer(GL_ARRAY_BUFFER, instance_data_buffer); // Instances

		glDrawElements(
			GL_TRIANGLES,
			baked_geometry.m_instances_count,
			baked_geometry.m_indices_type,
			NULL
		);
	}
}

Renderer* Renderer::create()
{
	return new RendererGL();
}
