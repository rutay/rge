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
    vec3 position;
    vec3 color;
    uint type;
    vec4 data[2];
};

layout(std140) uniform LightBuffer
{
    uint lights_count;
    Light lights[MAX_LIGHT_BUFFER_SIZE];
};

#define PointLight_radius(_light) _light.data[0].x

#define DirectionalLight_direction(_light) _light.data[0].xyz

#define SpotLight_direction(_light) _light.data[0].xyz
#define SpotLight_angle(_light)     _light.data[0].w
#define SpotLight_distance(_light)  _light.data[1].x

bool PointLight_affects(Light light, vec3 position, vec3 normal, out vec3 light_vector)
{
	if (distance(position, light.position) < PointLight_radius(light))
	{
		light_vector = light.position - position;
		return true;
	}
	else
	{
		return false;
	}
}

bool DirectionalLight_affects(Light light, vec3 position, vec3 normal, out vec3 light_vector)
{
	light_vector = DirectionalLight_direction(light);
    return true;
}

bool SpotLight_affects(Light light, vec3 position, vec3 normal, out vec3 light_vector)
{	
    return true;
}

//
// The apply_light function depends on the material used, the material implementation (.mat file) must have a function with the following prototype:
//
// * vec3 apply_light(Light light, vec3 position, vec3 normal, vec3 light_vector)
//
// Eventually a material could add its own uniforms and use them.
//

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
		
		if (
			(light.type == LIGHT_TYPE_POINT_LIGHT       && PointLight_affects(light, position, normal, light_vector))       ||
			(light.type == LIGHT_TYPE_DIRECTIONAL_LIGHT && DirectionalLight_affects(light, position, normal, light_vector)) ||
			(light.type == LIGHT_TYPE_SPOT_LIGHT        && SpotLight_affects(light, position, normal, light_vector))        
		)
		{
			res_color += apply_light(light, position, normal, light_vector);
		}
    }

	// TODO Tone mapping to normalize the color values

    f_color = vec4(res_color, 1);
}
