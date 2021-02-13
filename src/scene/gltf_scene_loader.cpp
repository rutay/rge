#include "gltf_scene_loader.hpp"

#include <stdio.h>

#include "resource/resource_provider.hpp"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include <bx/math.h>

using namespace rge;

// TODO load using a more lightweight loader (tinygltf heavily uses C++ stl).

AccessorBuffer* GltfSceneLoader::create_buffer_from_accessor(tinygltf::Model& gltf_model, int accessor_idx)
{
    if (m_buffer_by_accessor_idx.contains(accessor_idx))
        return m_buffer_by_accessor_idx.at(accessor_idx);

    AccessorBuffer* result = new AccessorBuffer();

    tinygltf::Accessor& gltf_accessor = gltf_model.accessors[accessor_idx];

    result->m_component_type = static_cast<ComponentType>(gltf_accessor.componentType);
    result->m_component_size = tinygltf::GetComponentSizeInBytes(gltf_accessor.componentType);
    result->m_num_components = tinygltf::GetNumComponentsInType(gltf_accessor.type);
    result->m_count = gltf_accessor.count;

    tinygltf::BufferView& gltf_buffer_view = gltf_model.bufferViews[gltf_accessor.bufferView];
    tinygltf::Buffer& gltf_buffer = gltf_model.buffers[gltf_buffer_view.buffer];

    size_t byte_offset = gltf_buffer_view.byteOffset;
    size_t byte_length = gltf_buffer_view.byteLength;
    size_t byte_stride = gltf_buffer_view.byteStride > 0 ? gltf_buffer_view.byteStride : result->get_data_size();

    uint8_t* data = gltf_buffer.data.data();
    data += byte_offset;

    // The whole buffer_view is copied to the Buffer, keeping the data interleaved even if a Buffer must be used for just one accessor.
    // For example a single buffer_view could contain position and normal vertices interleaved.
    // For the position and normal accessor, separated, the buffer_view is copied to a new Buffer,
    // and the data is kept interleaved for both (not to iterate every single component and copy it away).

    result->m_data.resize(byte_length);
    std::copy(data, data + byte_length, result->m_data.data());

    result->m_stride = byte_stride;
    result->m_normalized = gltf_accessor.normalized;

    m_buffer_by_accessor_idx.insert({ accessor_idx, result });

    return result;
}

Material* GltfSceneLoader::load_material(tinygltf::Model& gltf_model, int material_idx)
{
    if (m_material_by_idx.contains(material_idx))
        return m_material_by_idx.at(material_idx);

    Material* result = new Material();

    tinygltf::Material& gltf_material = gltf_model.materials[material_idx];

    m_material_by_idx.insert({ material_idx, result });

    return result;
}

Mesh* GltfSceneLoader::load_mesh(tinygltf::Model& gltf_model, int mesh_idx)
{
    if (m_mesh_by_idx.contains(mesh_idx))
        return m_mesh_by_idx.at(mesh_idx);

    Mesh* result = new Mesh();

    tinygltf::Mesh& gltf_mesh = gltf_model.meshes[mesh_idx];
    tinygltf::Primitive& gltf_primitive = gltf_mesh.primitives[0]; // Atm always takes the first primitive.

    int material_idx = gltf_primitive.material;
    result->m_material = material_idx >= 0 ? load_material(gltf_model, material_idx) : NULL;

    for (auto [attrib_name, accessor_idx] : gltf_primitive.attributes)
    {
        AttribType attrib_type;

        if      (attrib_name == "POSITION")   attrib_type = AttribType::POSITION;
        else if (attrib_name == "NORMAL")     attrib_type = AttribType::NORMAL;
        else if (attrib_name == "TANGENT")    attrib_type = AttribType::TANGENT;
        else if (attrib_name == "TEXCOORD_0") attrib_type = AttribType::TEXCOORD_0;
        else if (attrib_name == "TEXCOORD_1") attrib_type = AttribType::TEXCOORD_1;
        else if (attrib_name == "COLOR_0")    attrib_type = AttribType::COLOR_0;
        else if (attrib_name == "JOINTS_0")   attrib_type = AttribType::JOINTS_0;
        else if (attrib_name == "WEIGHTS_0")  attrib_type = AttribType::WEIGHTS_0;
        else
        {
            printf("Invalid attribute name: %s\n", attrib_name.c_str());
            continue;
        }

        result->m_attributes[attrib_type] = accessor_idx >= 0 ? create_buffer_from_accessor(gltf_model, accessor_idx) : NULL;
    }

    int accessor_idx = gltf_primitive.indices;
    result->m_indices = accessor_idx >= 0 ? create_buffer_from_accessor(gltf_model, accessor_idx) : NULL;

    m_mesh_by_idx.insert({ mesh_idx, result });

    return result;
}

Node* GltfSceneLoader::load_node(tinygltf::Model& gltf_model, int node_idx, Node* parent)
{
    if (m_node_by_idx.contains(node_idx))
        return m_node_by_idx.at(node_idx);

    Node* result = new Node();

    tinygltf::Node& gltf_node = gltf_model.nodes[node_idx];

    int mesh_idx = gltf_node.mesh;
    result->m_mesh = mesh_idx >= 0 ? load_mesh(gltf_model, mesh_idx) : nullptr;

	if (!gltf_node.translation.empty()) {
		for (int i = 0; i < 3; i++) {
			result->m_position[i] = (float) gltf_node.translation[i];
		}
	}

	if (!gltf_node.rotation.empty()) {
		for (int i = 0; i < 4; i++) {
			result->m_rotation[i] = (float) gltf_node.rotation[i];
		}
	}

    if (!gltf_node.scale.empty()) {
		for (int i = 0; i < 3; i++) {
			result->m_scale[i] = (float) gltf_node.scale[i];
		}
    }

	result->update_local_transform();

	if (parent)
	{
		result->m_parent = parent;
		parent->m_children.push_back(result);

		result->update_world_transform();
	}

	// TODO light

    for (int child_node_idx : gltf_node.children)
        load_node(gltf_model, child_node_idx, result);

    m_node_by_idx.insert({ node_idx, result });

    return result;
}

Node* GltfSceneLoader::load_scene(tinygltf::Model& gltf_model, int scene_idx)
{
    if (m_scene_by_idx.contains(scene_idx))
        return m_scene_by_idx.at(scene_idx);

    Node* result = new Node();

    tinygltf::Scene gltf_scene = gltf_model.scenes[scene_idx];

    for (int child_node_idx : gltf_scene.nodes)
        load_node(gltf_model, child_node_idx, result);

    m_scene_by_idx.insert({ scene_idx, result });

    return result;
}

void GltfSceneLoader::clear_cache()
{
    m_buffer_by_accessor_idx.clear();
    m_mesh_by_idx.clear();
    m_material_by_idx.clear();
    m_node_by_idx.clear();
    m_scene_by_idx.clear();
}

void GltfSceneLoader::print_cache()
{
    printf("Scenes: %lld, Nodes: %lld, Meshes: %lld, Materials: %lld, Buffers: %lld\n",
        m_scene_by_idx.size(),
        m_node_by_idx.size(),
        m_mesh_by_idx.size(),
        m_material_by_idx.size(),
        m_buffer_by_accessor_idx.size()
   );
}

Node* GltfSceneLoader::load(tinygltf::Model& gltf_model)
{
    clear_cache();

    int scene_idx = gltf_model.defaultScene;
    Node* def_scene = scene_idx >= 0 ? load_scene(gltf_model, scene_idx) : NULL;

    print_cache();

    return def_scene;
}

Node* GltfSceneLoader::load_from_resource(char const* path)
{
    std::vector<uint8_t> buffer;
    ResourceProvider::request(path, buffer);

    tinygltf::TinyGLTF gltf_loader;
    tinygltf::Model gltf_model;
    std::string err_msg, warn_msg;
    gltf_loader.LoadBinaryFromMemory(&gltf_model, &err_msg, &warn_msg, buffer.data(), buffer.size());

    if (!err_msg.empty())
    {
        printf("ERROR: %s\n", err_msg.c_str());
        return NULL;
    }

    if (!warn_msg.empty())
    {
        printf("WARNING: %s\n", warn_msg.c_str());
        return NULL;
    }

    return load(gltf_model);
}
