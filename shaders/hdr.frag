layout (location = 0) out vec4 out_color;

in VS_OUT
{
  vec2 tex_coord;
} fs_in;

uniform sampler2D hdr_tex;
uniform sampler2D bloom_tex;

uniform bool is_bloom = true;
uniform float exposure = 1.0f;
uniform float bloom_intensity = 1.0f;

vec3 upsample_tent9(sampler2D tex, float lod, vec2 uv, vec2 texelSize, float radius)
{
	vec4 offset = texelSize.xyxy * vec4(1.0f, 1.0f, -1.0f, 0.0f) * radius;

	// Center
	vec3 result = textureLod(tex, uv, lod).rgb * 4.0f;

	result += textureLod(tex, uv - offset.xy, lod).rgb;
	result += textureLod(tex, uv - offset.wy, lod).rgb * 2.0;
	result += textureLod(tex, uv - offset.zy, lod).rgb;

	result += textureLod(tex, uv + offset.zw, lod).rgb * 2.0;
	result += textureLod(tex, uv + offset.xw, lod).rgb * 2.0;

	result += textureLod(tex, uv + offset.zy, lod).rgb;
	result += textureLod(tex, uv + offset.wy, lod).rgb * 2.0;
	result += textureLod(tex, uv + offset.xy, lod).rgb;

	return result * (1.0f / 16.0f);
}

// Based on http://www.oscars.org/science-technology/sci-tech-projects/aces
vec3 ACESTonemap(vec3 color)
{
	mat3 m1 = mat3(
		0.59719, 0.07600, 0.02840,
		0.35458, 0.90834, 0.13383,
		0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
		1.60475, -0.10208, -0.00327,
		-0.53108, 1.10813, -0.07276,
		-0.07367, -0.00605, 1.07602
	);
	vec3 v = m1 * color;
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);
}

void main()
{
	ivec2 bloom_tex_sizei = textureSize(bloom_tex, 0);
	vec2 bloom_tex_size = vec2(float(bloom_tex_sizei.x), float(bloom_tex_sizei.y));

	const float sample_scale = 0.5;
    vec3 bloom = vec3(0.0);
    if (is_bloom)
    {
        bloom = upsample_tent9(bloom_tex, 0, fs_in.tex_coord, 1.0f / bloom_tex_size, sample_scale) * bloom_intensity;
    }

    vec3 hdr_color = texture(hdr_tex, fs_in.tex_coord).rgb;
    hdr_color += bloom;
	hdr_color *= exposure;

	hdr_color = ACESTonemap(hdr_color);

    const float gamma = 2.2;

    vec3 mapped = hdr_color;
    // gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    out_color = vec4(mapped, 1.0);
}
