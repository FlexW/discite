layout(location = 0) in vec3 in_position;

out VS_OUT
{
    vec3 world_pos;
} vs_out;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

void main()
{
    vs_out.world_pos = in_position;

    mat4 rot_view_matrix = mat4(mat3(view_matrix));
    vec4 clip_pos = projection_matrix * rot_view_matrix * vec4(vs_out.world_pos, 1.0);

    gl_Position = clip_pos.xyww;
}
