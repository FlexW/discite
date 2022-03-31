in GEO_OUT
{
    vec2 tex_coord;
} fs_in;

uniform sampler2D tex;

void main()
{
    float alpha = texture(tex, fs_in.tex_coord).a;
    if (alpha <= 0.01f)
    {
        discard;
    }
}
