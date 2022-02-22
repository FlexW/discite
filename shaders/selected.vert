#version 460 core

layout (location = 0) in vec3 in_position;


uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

void main()
{
  vec4 position = vec4(in_position, 1.0);
  vec4 P = view_matrix * model_matrix * position;
  gl_Position = projection_matrix * P;
}
