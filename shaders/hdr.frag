#version 460 core

layout (location = 0) out vec4 out_color;

in VS_OUT
{
  vec2 tex_coord;
} fs_in;

uniform sampler2D hdr_tex;
uniform float exposure = 1.0f;

void main()
{
    const float gamma = 2.2;
    vec3 hdr_color = texture(hdr_tex, fs_in.tex_coord).rgb;

    // exposure tone mapping
    // vec3 mapped = vec3(1.0) - exp(-hdr_color * exposure);
    // vec3 mapped = hdr_color / (color + vec3(1.0));
    vec3 mapped = hdr_color;

    // gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    out_color = vec4(mapped, 1.0);
}
