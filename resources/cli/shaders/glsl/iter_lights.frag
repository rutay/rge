#version 300 es

precision highp float;
precision highp int;

in vec3 v_position;
in vec3 v_normal;
in vec3 v_color_0;

uniform vec3 u_view_pos;

#define MAX_LIGHT_BUFFER_SIZE 128

struct Light
{
    vec3 color;

	// cone
	vec3 position;
	vec3 direction;
	vec3 angle;
	float distance;
};

layout(std140) uniform rge_b_lights
{
    Light lights[MAX_LIGHT_BUFFER_SIZE];
};

uniform int u_lights_count;

#material
// vec3 apply_light(vec3 view_pos, vec3 frag_pos, vec3 frag_norm, Light light)

out vec4 f_color;

void main()
{
    vec3 frag_pos = v_position;
    vec3 frag_norm = v_normal;
    vec3 frag_color_0 = v_color_0;

    vec3 res_color = vec3(0, 0, 0);

    for (int i = 0; i < u_lights_count; i++)
    {
	    //res_color = vec3(1.0, 1.0, 0);
	   	res_color += apply_light(u_view_pos, frag_pos, frag_norm, lights[i]);
    }

	// TODO Tone mapping to normalize the color values

    f_color = vec4(res_color, 1);
}
