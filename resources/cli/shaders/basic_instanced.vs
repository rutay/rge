$input a_position, a_normal, i_data0, i_data1, i_data2, i_data3
$output v_position, v_normal

#include <bgfx_shader.sh>

void main()
{
	mat4 model;
	model[0] = i_data0;
	model[1] = i_data1;
	model[2] = i_data2;
	model[3] = i_data3;

	vec4 world_position = instMul(model, vec4(a_position, 1.0));
	gl_Position = mul(u_viewProj, world_position);

	v_position = vec3(world_position);
	v_normal = a_normal;
}
