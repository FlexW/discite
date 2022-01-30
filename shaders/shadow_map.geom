#version 460 core

#define LIGHT_SPACE_MATRICES_COUNT 5

layout(triangles, invocations = 5) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 light_space_matrices[LIGHT_SPACE_MATRICES_COUNT];

void main()
{
    for (int i = 0; i < LIGHT_SPACE_MATRICES_COUNT; ++i)
    {
        gl_Position = light_space_matrices[gl_InvocationID] * gl_in[i].gl_Position;
        gl_Layer = gl_InvocationID;
        EmitVertex();
    }
    EndPrimitive();
}
