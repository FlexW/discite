#version 460 core

layout (location = 0) in vec3 in_position;

uniform mat4 model_matrix;

void main()
{
    gl_Position = model_matrix * vec4(in_position, 1.0f);
}
