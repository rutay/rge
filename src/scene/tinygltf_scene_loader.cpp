#include "tinygltf_scene_loader.hpp"

#include "resource_provider.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

using namespace rge::scene;

Buffer* SceneLoader_tinygltf::load_buffer(tinygltf::Model const& gltf_model, int buffer_idx)
{
	if (m_buffer_by_idx.contains(buffer_idx)) {
		return m_buffer_by_idx.at(buffer_idx);
	}

	tinygltf::Buffer const& gltf_buffer = gltf_model.buffers[buffer_idx];

	Buffer* buffer = new Buffer(); // TODO maybe keep track of the instantiated stuff in rge?
	size_t data_size = gltf_buffer.data.size();
	buffer->m_byte_length = data_size;
	buffer->m_data = malloc(data_size);
	memcpy(buffer->m_data, gltf_buffer.data.data(), data_size);

	m_buffer_by_idx.insert({buffer_idx, buffer});

	return buffer;
}

BufferView* SceneLoader_tinygltf::load_buffer_view(tinygltf::Model const& gltf_model, int buffer_view_idx)
{
	if (m_buffer_view_by_idx.contains(buffer_view_idx)) {
		return m_buffer_view_by_idx.at(buffer_view_idx);
	}

	tinygltf::BufferView const& gltf_buffer_view = gltf_model.bufferViews[buffer_view_idx];

	BufferView* buffer_view = new BufferView();
	buffer_view->m_byte_length = gltf_buffer_view.byteLength;
	buffer_view->m_byte_offset = gltf_buffer_view.byteOffset;
	buffer_view->m_byte_stride = gltf_buffer_view.byteStride;

	if (gltf_buffer_view.buffer >= 0) {
		buffer_view->m_buffer = load_buffer(gltf_model, gltf_buffer_view.buffer);
	}

	m_buffer_view_by_idx.insert({buffer_view_idx, buffer_view});

	return buffer_view;
}

Accessor* SceneLoader_tinygltf::load_accessor(tinygltf::Model const& gltf_model, int accessor_idx)
{
	if (m_accessor_by_idx.contains(accessor_idx)) {
		return m_accessor_by_idx.at(accessor_idx);
	}

	tinygltf::Accessor const& gltf_accessor = gltf_model.accessors[accessor_idx];

	Accessor* accessor = new Accessor();
	accessor->m_component_type = static_cast<ComponentType>(gltf_accessor.componentType);
	accessor->m_num_components = tinygltf::GetNumComponentsInType(gltf_accessor.type);
	accessor->m_normalized = gltf_accessor.normalized;
	accessor->m_byte_offset = gltf_accessor.byteOffset;
	accessor->m_count = gltf_accessor.count;
	accessor->m_normalized = gltf_accessor.normalized;;

	if (gltf_accessor.bufferView >= 0) {
		accessor->m_buffer_view = load_buffer_view(gltf_model, gltf_accessor.bufferView);
	}

	m_accessor_by_idx.insert({accessor_idx, accessor});

	return accessor;
}

Material* SceneLoader_tinygltf::load_material(tinygltf::Model const& gltf_model, int material_idx)
{
	if (m_material_by_idx.contains(material_idx))
		return m_material_by_idx.at(material_idx);

	BasicMaterial* material = new BasicMaterial(); // todo
	material->m_color[0] = 1.0f;
	material->m_color[3] = 1.0f;

	tinygltf::Material const& gltf_material = gltf_model.materials[material_idx];

	// TODO

	m_material_by_idx.insert({material_idx, material});

	return material;
}

Mesh* SceneLoader_tinygltf::load_primitive(tinygltf::Model const& gltf_model, int mesh_idx, tinygltf::Primitive const& gltf_primitive)
{
	if (m_mesh_by_idx.contains(mesh_idx))
		return m_mesh_by_idx.at(mesh_idx);

	Mesh* mesh = new Mesh();

	// Material
	mesh->m_material = load_material(gltf_model, gltf_primitive.material);

	// Geometry
	Geometry* geometry = new Geometry();
	geometry->m_mode = static_cast<DrawMode>(gltf_primitive.mode);

	for (auto[attrib_name, accessor_idx] : gltf_primitive.attributes) {
		if (accessor_idx < 0)
			continue;

		AttribType attrib_type;
		if (attrib_name == "POSITION")        attrib_type = AttribType::POSITION;
		else if (attrib_name == "NORMAL")     attrib_type = AttribType::NORMAL;
		else if (attrib_name == "TANGENT")    attrib_type = AttribType::TANGENT;
		else if (attrib_name == "TEXCOORD_0") attrib_type = AttribType::TEXCOORD_0;
		else if (attrib_name == "TEXCOORD_1") attrib_type = AttribType::TEXCOORD_1;
		else if (attrib_name == "COLOR_0")    attrib_type = AttribType::COLOR_0;
		else if (attrib_name == "JOINTS_0")   attrib_type = AttribType::JOINTS_0;
		else if (attrib_name == "WEIGHTS_0")  attrib_type = AttribType::WEIGHTS_0;
		else {
			printf("Invalid attribute name: %s\n", attrib_name.c_str());
			continue;
		}

		geometry->m_attributes[static_cast<size_t>(attrib_type)] = load_accessor(gltf_model, accessor_idx);
	}

	if (gltf_primitive.indices >= 0) {
		geometry->m_indices = load_accessor(gltf_model, gltf_primitive.indices);
	}

	mesh->m_geometry = geometry;

	m_mesh_by_idx.insert({mesh_idx, mesh});

	return mesh;
}

void SceneLoader_tinygltf::load_mesh(tinygltf::Model const& gltf_model, int mesh_idx, std::vector<Mesh*>& meshes)
{
	auto gltf_mesh = gltf_model.meshes[mesh_idx];
	for (tinygltf::Primitive& gltf_primitive : gltf_mesh.primitives) {
		meshes.push_back(
			load_primitive(gltf_model, mesh_idx, gltf_primitive)
		);
	}
}

Node* SceneLoader_tinygltf::load_node(tinygltf::Model const& gltf_model, int node_idx, Node* parent)
{
	if (m_node_by_idx.contains(node_idx))
		return m_node_by_idx.at(node_idx);

	Node* node = new Node();

	tinygltf::Node const& gltf_node = gltf_model.nodes[node_idx];

	if (gltf_node.mesh >= 0) {
		load_mesh(gltf_model, gltf_node.mesh, node->m_meshes);
	}

	if (!gltf_node.translation.empty()) {
		for (int i = 0; i < 3; i++) {
			node->m_position[i] = (float)gltf_node.translation[i];
		}
	}

	if (!gltf_node.rotation.empty()) {
		for (int i = 0; i < 4; i++) {
			node->m_rotation[i] = (float)gltf_node.rotation[i];
		}
	}

	if (!gltf_node.scale.empty()) {
		for (int i = 0; i < 3; i++) {
			node->m_scale[i] = (float)gltf_node.scale[i];
		}
	}

	node->update_local_transform();

	if (parent) {
		node->m_parent = parent;
		parent->m_children.push_back(node);

		node->update_world_transform();
	}

	// TODO light

	for (int child_node_idx : gltf_node.children) {
		load_node(gltf_model, child_node_idx, node);
	}

	m_node_by_idx.insert({node_idx, node});

	return node;
}

Node* SceneLoader_tinygltf::load_scene(tinygltf::Model const& gltf_model, int scene_idx)
{
	if (m_scene_by_idx.contains(scene_idx))
		return m_scene_by_idx.at(scene_idx);

	Node* scene = new Node();

	tinygltf::Scene gltf_scene = gltf_model.scenes[scene_idx];

	for (int child_node_idx : gltf_scene.nodes)
		load_node(gltf_model, child_node_idx, scene);

	m_scene_by_idx.insert({scene_idx, scene});

	return scene;
}

void SceneLoader_tinygltf::clear_cache()
{
	m_buffer_by_idx.clear();
	m_buffer_view_by_idx.clear();
	m_accessor_by_idx.clear();
	m_mesh_by_idx.clear();
	m_material_by_idx.clear();
	m_node_by_idx.clear();
	m_scene_by_idx.clear();
}

Node* SceneLoader_tinygltf::load(tinygltf::Model& gltf_model)
{
	clear_cache();

	Node* def_scene;
	if (gltf_model.defaultScene >= 0) {
		def_scene = load_scene(gltf_model, gltf_model.defaultScene);
	} else {
		return nullptr;
	}
	return def_scene;
}

Node* SceneLoader_tinygltf::load_from_resource(char const* path)
{
	std::vector<uint8_t> buffer;
	rge::resource_provider::read(path, buffer);

	tinygltf::TinyGLTF gltf_loader;
	tinygltf::Model gltf_model;
	std::string err_msg, warn_msg;
	gltf_loader.LoadBinaryFromMemory(&gltf_model, &err_msg, &warn_msg, buffer.data(), buffer.size());

	if (!err_msg.empty()) {
		printf("ERROR: %s\n", err_msg.c_str()); // todo throw error
		return nullptr;
	}

	if (!warn_msg.empty()) {
		printf("WARNING: %s\n", warn_msg.c_str()); // todo throw error
		return nullptr;
	}

	return load(gltf_model);
}
