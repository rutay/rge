#version 300 es

#include "v_attr.glsl"

DEF_a_position;
DEF_a_normal;
DEF_a_color_0;
DEF_i_model;

uniform mat4 u_camera_view;
uniform mat4 u_camera_projection;

out vec3 v_position;
out vec3 v_normal;
out vec3 v_color_0;

void main()
{
    vec4 world_pos = i_model * a_position;

    gl_Position = u_camera_projection * u_camera_view * world_pos;

    v_position = vec3(world_pos);
    v_normal = vec3(a_normal);
    v_color_0 = vec3(a_color_0);
}
