in VS_OUT
{
    vec3 world_pos;
} fs_in;

layout (location = 0) out vec4 out_color;

uniform samplerCube env_tex;

void main()
{
    vec3 env_color = texture(env_tex, fs_in.world_pos).rgb;

    out_color = vec4(env_color, 1.0);
}
