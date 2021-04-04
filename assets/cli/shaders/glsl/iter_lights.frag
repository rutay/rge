#version 300 es

in vec3 v_position;
in vec3 v_normal;
in vec3 v_color;

#define MAX_LIGHT_BUFFER_SIZE 128

#define LIGHT_TYPE_POINT_LIGHT       0
#define LIGHT_TYPE_DIRECTIONAL_LIGHT 1
#define LIGHT_TYPE_SPOT_LIGHT        2

struct Light
{
    vec3 color;

	// cone
	vec3 position;
	vec3 direction;
	vec3 angle;
	float distance;
};

layout(std140) uniform LightBuffer
{
    uint lights_count;
    Light lights[MAX_LIGHT_BUFFER_SIZE];
};

#material

out vec4 f_color;

void main()
{
    vec3 position = v_position;
    vec3 normal   = v_normal;
    vec3 color    = v_color;

    vec3 res_color = vec3(0);

    for (int i = 0; i < lights_count; i++)
    {
        Light light = lights[i];
		vec3 light_vector;

		res_color += apply_light(light, position, normal, light_vector);
    }

	// TODO Tone mapping to normalize the color values

    f_color = vec4(res_color, 1);
}
