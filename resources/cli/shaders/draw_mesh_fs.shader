$input v_position, v_normal

uniform vec4 u_material_data0;
uniform vec4 u_material_data1;

#define u_albedo    u_material_data0.xyz
#define u_metallic  u_material_data0.w
#define u_roughness u_material_data1.x
#define u_ao        u_material_data1.y

void main()
{
	//gl_FragData[0] = vec4(v_position, 1.0);
	//gl_FragData[1] = vec4(v_normal, 1.0);
	//gl_FragData[2] = u_material_data0;
	//gl_FragData[3] = u_material_data1;

	gl_FragColor = vec4(1, 0, 0, 1);
}
