
#define MATERIAL_TYPE_BASIC_MATERIAL 0
#define MATERIAL_TYPE_PHONG_MATERIAL 1
#define MATERIAL_TYPE_PBR_MATERIAL   2

#define DEFINE_MATERIAL_BUFFER(_name) layout (std140) uniform _name

struct Material
{
	int type;
};

#define MATERIAL_IMPL_DATA_SIZE 4 // Size expressed in vec4

#define DEFINE_MATERIALS_BUFFER(name, max_count) \
	uniform vec4 name[max_count]; \
	uniform vec4 name##_impl_data[max_count * MATERIAL_IMPL_DATA_SIZE]; \
	uniform vec4 name##count

struct BasicMaterial
{
	vec3 color;
};

#define BASIC_MATERIAL(_buffer, i) \
	BasicMaterial(_buffer##_impl_data[i + 0].xyz)

struct PhongMaterial
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

#define PHONG_MATERIAL(_buffer, i) \
	PhongMaterial(          \
		_buffer##_impl_data[i + 0].xyz, \
		_buffer##_impl_data[i + 1].xyz, \
		_buffer##_impl_data[i + 2].xyz, \
		_buffer##_impl_data[i + 2].w    \
	)
