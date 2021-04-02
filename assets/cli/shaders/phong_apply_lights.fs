$input v_position, v_normal

#include "lights.s"
#include "materials.s"

DEFINE_LIGHT_BUFFER(u_lights_buffer, 1);
DEFINE_MATERIALS_BUFFER(u_material, 1);

vec3 PointLight_apply(Light light, PointLight point_light, vec3 position, vec3 normal, PhongMaterial material)
{
	return vec3(1, 0, 0);
}

vec3 DirectionalLight_apply(Light light, DirectionalLight directional_light, vec3 position, vec3 normal, PhongMaterial material)
{
	return vec3(0, 1, 0);
}

vec3 SpotLight_apply(Light light, SpotLight spot_light, vec3 position, vec3 normal, PhongMaterial material)
{
	return vec3(0, 0, 1);
}

void main()
{
	PhongMaterial material = PHONG_MATERIAL(u_material, 0);

	vec3 color = vec3(0);

	for (int i = 0; i < u_lights_count; i++)
	{
		Light light = LIGHT(i);

		if (light.type == LIGHT_TYPE_POINT_LIGHT)
		{
			color += PointLight_apply(
				light,
				POINT_LIGHT(i),
				v_position,
				v_normal,
				material
			);
		}
		else if (light.type == LIGHT_TYPE_DIRECTIONAL_LIGHT)
		{
			color += DirectionalLight_apply(
				light,
				DIRECTIONAL_LIGHT(i),
				v_position,
				v_normal,
				material
			);
		}
		else if (light.type == LIGHT_TYPE_SPOT_LIGHT)
		{
			color += SpotLight_apply(
				light,
				SPOT_LIGHT(i),
				v_position,
				v_normal,
				material
			);
		}
	}

	gl_FragColor = vec4(color, 1);
}
