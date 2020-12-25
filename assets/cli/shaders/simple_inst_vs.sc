

$input a_position, a_color0, a_normal, i_data0, i_data1, i_data2, i_data3
$output v_position, v_color0, v_normal

void main()
{
	mat4 model;
	model[0] = i_data0;
	model[1] = i_data1;
	model[2] = i_data2;
	model[3] = i_data3;

	vec4 worldPos = instMul(model, vec4(a_position, 1.0));
	gl_Position = mul(u_viewProj, worldPos);

	v_position = a_position;
	v_color0 = a_color0;
	v_normal = a_normal;
}
