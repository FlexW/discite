#version 460 core

#define MAX_BONES 35

#ifdef SKINNED
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_tangent;
layout(location = 3) in vec3 in_bitangent;
layout(location = 4) in ivec4 in_skin_bones;
layout(location = 5) in vec4 in_skin_weights;
layout(location = 6) in vec2 in_tex_coord;
#else // SKINNED
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_tangent;
layout (location = 3) in vec3 in_bitangent;
layout (location = 4) in vec2 in_tex_coord;
#endif // SKINNED

out VS_OUT
{
  vec3 position;
  vec3 normal;
  vec3 tangent;
  vec3 bitangent;
  vec2 tex_coord;
} vs_out;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

uniform mat4 model_matrix;
#ifdef SKINNED
uniform mat4 bones[MAX_BONES];
#endif // SKINNED

void main()
{
  #ifdef SKINNED
  mat4 bone_transform = bones[in_skin_bones.x] * in_skin_weights.x;
  bone_transform += bones[in_skin_bones.y] * in_skin_weights.y;
  bone_transform += bones[in_skin_bones.z] * in_skin_weights.z;
  bone_transform += bones[in_skin_bones.w] * in_skin_weights.w;

  vec4 position = bone_transform * vec4(in_position, 1.0);
  vec3 normal = vec3(bone_transform * vec4(in_normal, 0.0));
  vec3 tangent = vec3(bone_transform * vec4(in_tangent, 0.0));
  vec3 bitangent = vec3(bone_transform * vec4(in_bitangent, 0.0));
  #else // SKINNED
  vec4 position = vec4(in_position, 1.0);
  vec3 normal = in_normal;
  vec3 tangent = in_tangent;
  vec3 bitangent = in_bitangent;
  #endif // SKINNED

  mat4 view_model_matrix = view_matrix * model_matrix;

  vec4 P = view_model_matrix * position;
  vec3 N = normalize(transpose(inverse(mat3(view_model_matrix))) * normal);
  vec3 T = normalize(transpose(inverse(mat3(view_model_matrix))) * tangent);
  vec3 B = normalize(transpose(inverse(mat3(view_model_matrix))) * bitangent);
  T = normalize(T - dot(T, N) * N);
  B = cross(N, T);

  vs_out.position = P.xyz;
  vs_out.normal = N;
  vs_out.tangent = T;
  vs_out.bitangent = B;
  vs_out.tex_coord = in_tex_coord;

  gl_Position = projection_matrix * P;
}
