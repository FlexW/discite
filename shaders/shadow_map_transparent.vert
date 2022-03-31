#define MAX_BONES 35

#ifdef SKINNED
layout (location = 0) in vec3 in_position;
layout (location = 4) in ivec4 in_skin_bones;
layout (location = 5) in vec4 in_skin_weights;
layout (location = 6) in vec2 in_tex_coord;
#else // SKINNED
layout (location = 0) in vec3 in_position;
layout (location = 4) in vec2 in_tex_coord;
#endif // SKINNED

out VS_OUT
{
    vec2 tex_coord;
} vs_out;

uniform mat4 model_matrix;

void main()
{
    vs_out.tex_coord = in_tex_coord;

#ifdef SKINNED
    mat4 bone_transform = bones[in_skin_bones.x] * in_skin_weights.x;
    bone_transform += bones[in_skin_bones.y] * in_skin_weights.y;
    bone_transform += bones[in_skin_bones.z] * in_skin_weights.z;
    bone_transform += bones[in_skin_bones.w] * in_skin_weights.w;

    gl_Position = model_matrix * bone_transform * vec4(in_position, 1.0);
#else // SKINNED
    gl_Position = model_matrix * vec4(in_position, 1.0f);
#endif // SKINNED
}
