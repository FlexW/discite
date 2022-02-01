#version 460 core

layout (location = 0) in vec3 in_position;
layout (location = 4) in vec2 in_tex_coord;

out VS_OUT
{
    vec2 tex_coord;
} vs_out;

uniform mat4 model_matrix;

void main()
{
    vs_out.tex_coord = in_tex_coord;
    gl_Position = model_matrix * vec4(in_position, 1.0f);
}
