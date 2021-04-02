#version 300 es

in vec3 position;
in vec3 normal;
in vec3 color;

uniform mat4 u_camera_view, u_camera_projection;

out vec3 v_position;
out vec3 v_normal;
out vec3 v_color;

void main()
{
    vec4 world_position = u_model * vec4(position, 1);

    gl_Position = u_projection * u_view * world_position;

    v_position = vec3(world_position);
    v_normal = normal;
    v_color = color;
}