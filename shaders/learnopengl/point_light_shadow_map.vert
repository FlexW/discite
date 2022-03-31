#define MAX_BONES 35

#ifdef SKINNED
layout (location = 0) in vec3 in_position;
layout (location = 4) in ivec4 in_skin_bones;
layout (location = 5) in vec4 in_skin_weights;
#else // SKINNED
layout (location = 0) in vec3 in_position;
#endif // SKINNED

uniform mat4 model;
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

    gl_Position = model * bone_transform * vec4(in_position, 1.0);
#else // SKINNED
    gl_Position = model * vec4(in_position, 1.0f);
#endif // SKINNED
}
