#version 460 core

in VS_OUT
{
    vec3 position;
    vec3 position_world_space;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 tex_coord;
} fs_in;

uniform sampler2D tex;
uniform bool is_tex;

void main()
{
    if (is_tex)
    {
        float alpha = texture(tex, fs_in.tex_coord).a;
        if (alpha <= 0.01f)
        {
            discard;
        }
    }
}
