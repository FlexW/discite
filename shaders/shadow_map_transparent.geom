#version 460 core

#define LIGHT_SPACE_MATRICES_COUNT 4

layout(triangles, invocations = LIGHT_SPACE_MATRICES_COUNT) in;
layout(triangle_strip, max_vertices = 3) out;

in VS_OUT
{
    vec2 tex_coord;
} geo_in[];

out GEO_OUT
{
    vec2 tex_coord;
} geo_out;

uniform mat4 light_space_matrices[LIGHT_SPACE_MATRICES_COUNT];

void main()
{

    for (int i = 0; i < LIGHT_SPACE_MATRICES_COUNT; ++i)
    {
        geo_out.tex_coord = geo_in[i].tex_coord;
        gl_Position = light_space_matrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
