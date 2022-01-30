#version 460 core

layout (location = 0) out vec4 out_color;

in VS_OUT
{
  vec2 tex_coord;
} fs_in;

uniform sampler2DArray depth_tex;
uniform float near_plane;
uniform float far_plane;
uniform int layer;

float linerize_depth(float depth)
{
    float z = depth * 2.0 - 1; // back to ndc
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
    float depth_value = texture(depth_tex, vec3(fs_in.tex_coord, layer)).r;
    // frag_color = vec4(vec3(linear_depth(depth_value) / far_plane), 1.0); // perspective
    out_color = vec4(vec3(depth_value), 1.0); // orthographic
}

