#version 460 core

#define MAX_POINT_LIGHTS_COUNT 5
#define CASCADES_COUNT 5

const float PI = 3.141592653589793;
const float epsilon = 0.00001;

in VS_OUT
{
    vec3 position;
    vec3 position_world_space;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 tex_coord;
} fs_in;

layout (location = 0) out vec4 out_color;

struct PointLight
{
    // in view space
    vec3 position;

    vec3 color;
    float multiplier;

    float radius;
    float falloff;
};

struct DirectionalLight
{
    // in view space
    vec3 direction;
    vec3 radiance;
    float multiplier;

    vec3 color;
};

uniform bool directional_light_shadow_enabled = false;
uniform sampler2DArray directional_light_shadow_tex;
uniform mat4 light_space_matrices[CASCADES_COUNT];
uniform float cascades_plane_distances[CASCADES_COUNT - 1];
uniform bool show_shadow_cascades = false;

uniform sampler2D albedo_tex;
uniform vec3 albedo_color;

uniform sampler2D metalness_roughness_tex;
uniform float roughness;
uniform float metalness;

uniform sampler2D ao_tex;
uniform bool ao_tex_enabled = false;

uniform sampler2D emissive_tex;
uniform vec3 emissive;

uniform sampler2D normal_tex;
uniform bool normal_tex_enabled = false;

uniform samplerCube env_tex;
uniform samplerCube env_irradiance_tex;
uniform sampler2D brdf_lut_tex;

uniform int point_light_count = 0;
uniform PointLight point_lights[MAX_POINT_LIGHTS_COUNT];

uniform bool smooth_shadows = true;
uniform float light_size = 0.25f;
uniform float shadow_bias_min = 0.003f;
uniform bool directional_light_enabled = false;
uniform DirectionalLight directional_light;

////////////////////////////////////////////////////////////////////////////////
/// Shadows
////////////////////////////////////////////////////////////////////////////////

const float shadow_fade = 1.0;

float calc_shadow_bias(uint layer)
{
    float bias = max(shadow_bias_min
                     * (tan(acos(dot(fs_in.normal, directional_light.direction)))),
                     shadow_bias_min);

    bias *= 1.0 / (cascades_plane_distances[layer] * 0.5f);
    return bias;
}

float search_region_radius_uv(float z_world)
{
    const float light_z_near = 0.0;
    const float light_radius_uv = 0.05;
    return light_radius_uv * (z_world - light_z_near) / z_world;
}

const int POISSON_DISTRIBUTION_COUNT = 64;
const vec2 poisson_distribution[POISSON_DISTRIBUTION_COUNT] = vec2[](
	vec2(-0.884081, 0.124488),
	vec2(-0.714377, 0.027940),
	vec2(-0.747945, 0.227922),
	vec2(-0.939609, 0.243634),
	vec2(-0.985465, 0.045534),
	vec2(-0.861367, -0.136222),
	vec2(-0.881934, 0.396908),
	vec2(-0.466938, 0.014526),
	vec2(-0.558207, 0.212662),
	vec2(-0.578447, -0.095822),
	vec2(-0.740266, -0.095631),
	vec2(-0.751681, 0.472604),
	vec2(-0.553147, -0.243177),
	vec2(-0.674762, -0.330730),
	vec2(-0.402765, -0.122087),
	vec2(-0.319776, -0.312166),
	vec2(-0.413923, -0.439757),
	vec2(-0.979153, -0.201245),
	vec2(-0.865579, -0.288695),
	vec2(-0.243704, -0.186378),
	vec2(-0.294920, -0.055748),
	vec2(-0.604452, -0.544251),
	vec2(-0.418056, -0.587679),
	vec2(-0.549156, -0.415877),
	vec2(-0.238080, -0.611761),
	vec2(-0.267004, -0.459702),
	vec2(-0.100006, -0.229116),
	vec2(-0.101928, -0.380382),
	vec2(-0.681467, -0.700773),
	vec2(-0.763488, -0.543386),
	vec2(-0.549030, -0.750749),
	vec2(-0.809045, -0.408738),
	vec2(-0.388134, -0.773448),
	vec2(-0.429392, -0.894892),
	vec2(-0.131597, 0.065058),
	vec2(-0.275002, 0.102922),
	vec2(-0.106117, -0.068327),
	vec2(-0.294586, -0.891515),
	vec2(-0.629418, 0.379387),
	vec2(-0.407257, 0.339748),
	vec2(0.071650, -0.384284),
	vec2(0.022018, -0.263793),
	vec2(0.003879, -0.136073),
	vec2(-0.137533, -0.767844),
	vec2(-0.050874, -0.906068),
	vec2(0.114133, -0.070053),
	vec2(0.163314, -0.217231),
	vec2(-0.100262, -0.587992),
	vec2(-0.004942, 0.125368),
	vec2(0.035302, -0.619310),
	vec2(0.195646, -0.459022),
	vec2(0.303969, -0.346362),
	vec2(-0.678118, 0.685099),
	vec2(-0.628418, 0.507978),
	vec2(-0.508473, 0.458753),
	vec2(0.032134, -0.782030),
	vec2(0.122595, 0.280353),
	vec2(-0.043643, 0.312119),
	vec2(0.132993, 0.085170),
	vec2(-0.192106, 0.285848),
	vec2(0.183621, -0.713242),
	vec2(0.265220, -0.596716),
	vec2(-0.009628, -0.483058),
	vec2(-0.018516, 0.435703)
	);


vec2 sample_poisson(int index)
{
    return poisson_distribution[index % POISSON_DISTRIBUTION_COUNT];
}

float find_blocker_distance_directional_light(sampler2DArray shadow_tex,
                                              uint cascade,
                                              vec3 shadow_coords)
{
    float bias = calc_shadow_bias(cascade);

    const int blocker_search_samples_count = 64;
    int blockers_count = 0;
    float avg_blocker_distance = 0;

    float search_width = search_region_radius_uv(shadow_coords.z);
    for (int i = 0; i < blocker_search_samples_count; ++i)
    {
        float z = texture(shadow_tex,
                          vec3((shadow_coords.xy) + sample_poisson(i) * search_width, cascade)).r;
        if (z < (shadow_coords.z - bias))
        {
            ++blockers_count;
            avg_blocker_distance += z;
        }
    }

    if (blockers_count > 0)
    {
        return avg_blocker_distance / float(blockers_count);
    }

    // no blockers found
    return -1;
}

float directional_light_pcf(sampler2DArray shadow_tex,
                            uint cascade,
                            vec3 shadow_coords,
                            float uv_radius)
{
    float bias = calc_shadow_bias(cascade);
    const int pcf_samples_count = 64;

    float sum = 0.0f;
    for (int i = 0; i < pcf_samples_count; ++i)
    {
        vec2 offset = sample_poisson(i) * uv_radius;
        float z = texture(shadow_tex,
                          vec3((shadow_coords.xy) + offset, cascade)).r;
        sum += step(shadow_coords.z - bias, z);
    }

    return sum / float(pcf_samples_count);
}

float directional_light_pcss(sampler2DArray shadow_tex,
                             uint cascade,
                             vec3 shadow_coords,
                             float uv_light_size)
{
    float blocker_distance = find_blocker_distance_directional_light(shadow_tex,
                                                                     cascade,
                                                                     shadow_coords);
    const float uv_radius_max = 0.005f;
    float uv_radius = uv_radius_max;
    if (blocker_distance != -1.0)
    {
        float penumbra_width = (shadow_coords.z - blocker_distance) / blocker_distance;

        const float near = 0.01;
        uv_radius = penumbra_width * uv_light_size * near / shadow_coords.z;
        uv_radius = min(uv_radius, uv_radius_max);
    }


    return directional_light_pcf(shadow_tex, cascade, shadow_coords, uv_radius);
}

int calc_cascade_index()
{
    float depth_value = abs(fs_in.position.z);

    int layer = CASCADES_COUNT - 1;
    for (int i = 0; i < CASCADES_COUNT - 1; ++i)
    {
        if (depth_value < cascades_plane_distances[i])
        {
            layer = i;
            break;
        }
    }

    return layer;
}

float calc_directional_light_shadow()
{
    int layer = calc_cascade_index();
    vec3 world_pos = fs_in.position_world_space;
    vec4 frag_pos_light_space = light_space_matrices[layer]
        * (vec4(world_pos, 1.0));
    vec3 proj_coords = frag_pos_light_space.xyz / frag_pos_light_space.w;
    proj_coords = proj_coords * 0.5 + 0.5;

    if (proj_coords.z > 1.0)
    {
        return 1.0;
    }

    float shadow = 1.0;
    if (smooth_shadows)
    {
        // PCSS
        shadow = directional_light_pcss(directional_light_shadow_tex,
                                        layer,
                                        proj_coords,
                                        light_size);

    }
    else
    {
        // hard shadows
        float bias = calc_shadow_bias(layer);

        // PCF
        shadow = 0.0;
        vec2 texel_size = 1.0 / vec2(textureSize(directional_light_shadow_tex, 0));
        const int kernel_size = 1;
        for (int x = -kernel_size; x <= kernel_size; ++x)
        {
            for (int y = -kernel_size; y <= kernel_size; ++y)
            {
                float pcf_depth = texture(directional_light_shadow_tex,
                                          vec3(proj_coords.xy + vec2(x, y) * texel_size, layer)).r;
                shadow += (proj_coords.z - bias) > pcf_depth ? 1.0 : 0.0;

            }
        }
        float d = pow(kernel_size * 2 + 1, 2);
        shadow /= d;

        shadow = 1.0 - shadow;
    }

    return shadow;
}

////////////////////////////////////////////////////////////////////////////////
// PBR based on:
// https://learnopengl.com/PBR/IBL/Specular-IBL
////////////////////////////////////////////////////////////////////////////////

vec4 srgb_to_linear(vec4 srgb_in)
{
	vec3 lin_out = pow(srgb_in.xyz, vec3(2.2));
	return vec4(lin_out, srgb_in.a);
}

struct PbrParameters
{
    vec3 albedo;
    float roughness;
    float metalness;

    vec3 emissive;
    vec3 ao;

    vec3 normal;
    vec3 view;
    float n_dot_v;
};

PbrParameters pbr_params;

void calc_albedo()
{
    vec4 tex = texture(albedo_tex, fs_in.tex_coord);
    if (tex.a <= 0.01f)
    {
        discard;
    }
    pbr_params.albedo = tex.rgb * albedo_color;
}

void calc_roughness_metalness()
{
    vec2 rm =  texture(metalness_roughness_tex, fs_in.tex_coord).gb;

    pbr_params.roughness = rm.x * roughness;
    // minimal roughness to keep specular highlight
    pbr_params.roughness = max(pbr_params.roughness, 0.05f);

    pbr_params.metalness = rm.y * metalness;
}

void calc_emissive()
{
    pbr_params.emissive = texture(emissive_tex, fs_in.tex_coord).rgb  * emissive;
}

void calc_ao()
{
    if (ao_tex_enabled)
    {
        pbr_params.ao = texture(ao_tex, fs_in.tex_coord).rgb;
    }
    else
    {
        pbr_params.ao = vec3(0.0f);
    }
}

void calc_normal()
{
    if (normal_tex_enabled)
    {
        mat3 TBN = mat3(fs_in.tangent, fs_in.bitangent, fs_in.normal);
        vec3 normal = texture(normal_tex, fs_in.tex_coord).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN * normal);
        pbr_params.normal = normal;
    }
    else
    {
        pbr_params.normal = normalize(fs_in.normal);
    }
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float ndf_ggx(float cos_lh, float roughness)
{
	float alpha = roughness * roughness;
	float alpha_sq = alpha * alpha;

	float denom = (cos_lh * cos_lh) * (alpha_sq - 1.0) + 1.0;
	return alpha_sq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float ga_schlick_g1(float cos_theta, float k)
{
	return cos_theta / (cos_theta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float ga_schlick_ggx(float cos_li, float n_dot_v, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return ga_schlick_g1(cos_li, k) * ga_schlick_g1(n_dot_v, k);
}

float geometry_schlick_ggx(float n_dot_v, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = n_dot_v;
	float denom = n_dot_v * (1.0 - k) + k;

	return nom / denom;
}

vec3 fresnel_schlick_roughness(vec3 f0, float cos_theta, float roughness)
{
    cos_theta = clamp(cos_theta, 0.0f, 1.0f);
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cos_theta, 5.0);
}

vec3 ibl(vec3 f0, vec3 lr)
{
	vec3 irradiance = texture(env_irradiance_tex, pbr_params.normal).rgb;
	vec3 f = fresnel_schlick_roughness(f0, pbr_params.n_dot_v, pbr_params.roughness);
	vec3 kd = (1.0 - f) * (1.0 - pbr_params.metalness);
	vec3 diffuse_ibl = pbr_params.albedo * irradiance;

    float mip_count = float(textureQueryLevels(env_tex));
    float nov = clamp(pbr_params.n_dot_v, 0.0, 1.0);
	vec3 r = 2.0 * dot(pbr_params.view, pbr_params.normal) * pbr_params.normal - pbr_params.view;

	vec3 specular_irradiance = textureLod(env_tex,
                                          -normalize(reflect(pbr_params.view, pbr_params.normal)),
                                          pbr_params.roughness * mip_count).rgb;

	vec2 brdf_sample_point = vec2(pbr_params.n_dot_v, 1.0 - pbr_params.roughness);
	vec2 specular_brdf = texture(brdf_lut_tex, brdf_sample_point).rg;
    vec3 specular_ibl = specular_irradiance * (f0 * specular_brdf.x + specular_brdf.y);

    return kd * diffuse_ibl + specular_ibl;
}

vec3 calc_directional_light(vec3 f0)
{
    if (!directional_light_enabled)
    {
        return vec3(0.0f);
    }

    vec3 li = normalize(-directional_light.direction); // Vector from surface point to light
    vec3 lradiance = directional_light.color * directional_light.multiplier;
    vec3 lh = normalize(li + pbr_params.view); // Half vector between both l and v

    float cos_li = clamp(dot(pbr_params.normal, li), 0.001, 1.0);
    float cos_lh = clamp(dot(pbr_params.normal, lh), 0.0, 1.0);

    // Calculate the shading terms for the microfacet specular shading model
    vec3 F = fresnel_schlick_roughness(f0, dot(lh, pbr_params.view), pbr_params.roughness);
    float D = ndf_ggx(cos_lh, pbr_params.roughness);
    float G = ga_schlick_ggx(cos_li, pbr_params.n_dot_v, pbr_params.roughness);

    vec3 kd = (1.0 - F) * (1.0 - pbr_params.metalness);
    vec3 diffuse_brdf = kd * pbr_params.albedo;

    // Cook-Torrance
    vec3 specular_brdf = (F * D * G) / max(epsilon, 4.0 * cos_li * pbr_params.n_dot_v);
    specular_brdf = clamp(specular_brdf, vec3(0.0f), vec3(10.0f));
    if (directional_light_shadow_enabled)
    {
        float shadow = calc_directional_light_shadow();
        diffuse_brdf *= shadow;
        specular_brdf *= shadow;
    }

    vec3 color = (diffuse_brdf + specular_brdf) * lradiance * cos_li;
    return color;
}

vec3 calc_point_lights(vec3 f0)
{
    vec3 result = vec3(0.0);
    for (int i = 0; i < point_light_count; ++i)
    {
        PointLight light = point_lights[i];
        vec3 li = normalize(light.position - fs_in.position);
        float light_distance = length(light.position - fs_in.position);
        vec3 lh = normalize(li + pbr_params.view);

        float attenuation = clamp(1.0
                                  - (light_distance * light_distance) / (light.radius * light.radius),
                                  0.0,
                                  1.0);
        attenuation *= mix(attenuation, 1.0, light.falloff);

        vec3 lradiance = light.color * light.multiplier * attenuation;

		// calculate angles between surface normal and various light vectors.
		float cos_li = max(0.0, dot(pbr_params.normal, li));
		float cos_lh = max(0.0, dot(pbr_params.normal, lh));

		vec3 F = fresnel_schlick_roughness(f0, dot(lh, pbr_params.view), pbr_params.roughness);
		float D = ndf_ggx(cos_lh, pbr_params.roughness);
		float G = ga_schlick_ggx(cos_li, pbr_params.n_dot_v, pbr_params.roughness);

		vec3 kd = (1.0 - F) * (1.0 - pbr_params.metalness);
		vec3 diffuse_brdf = kd * pbr_params.albedo;

		// Cook-Torrance
		vec3 specular_brdf = (F * D * G) / max(epsilon, 4.0 * cos_li * pbr_params.n_dot_v);
		specular_brdf = clamp(specular_brdf, vec3(0.0f), vec3(10.0f));

		result += (diffuse_brdf + specular_brdf) * lradiance * cos_li;
    }

    return result;
}

void main()
{
    calc_albedo();
    calc_normal();
    calc_roughness_metalness();
    calc_emissive();
    calc_ao();

    pbr_params.view = normalize(-fs_in.position);
    pbr_params.n_dot_v = min(max(dot(pbr_params.normal, pbr_params.view), 0.0), 1.0);

	// specular reflection vector
	vec3 lr = 2.0 * pbr_params.n_dot_v * pbr_params.normal - pbr_params.view;

	// fresnel reflectance, metals use albedo
    const vec3 fdielectric = vec3(0.04);
	vec3 f0 = mix(fdielectric, pbr_params.albedo, pbr_params.metalness);

    vec3 light_contribution = calc_directional_light(f0);
    light_contribution += calc_point_lights(f0);
    vec3 ibl_contribution = ibl(f0, lr);
    vec3 color = ibl_contribution + light_contribution;

    // ambient occlusion
    color = color * (pbr_params.ao.r < 0.01 ? 1.0 : pbr_params.ao.r);
    // emissive
    color += pbr_params.emissive.rgb;

    if (show_shadow_cascades)
    {
        vec3 cascade_color = vec3(1.0f);
        int cascade_index = calc_cascade_index();
        if (cascade_index == 0)
        {
            cascade_color = vec3(1.0f, 0.25f, 0.25f);
        }
        else if (cascade_index == 1)
        {
            cascade_color = vec3(0.25f, 1.0f, 0.25f);
        }
        else if (cascade_index == 2)
        {
            cascade_color = vec3(0.25f, 0.25f, 1.0f);
        }
        else if (cascade_index == 3)
        {
            cascade_color = vec3(0.25f, 1.0f, 1.0f);
        }
        else if (cascade_index == 4)
        {
            cascade_color = vec3(1.0f, 1.0f, 0.25f);
        }
        color *= cascade_color;
    }

    out_color = vec4(color, 1.0);
}
