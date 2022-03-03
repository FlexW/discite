#version 460 core

#define MAX_POINT_LIGHTS_COUNT 5
#define CASCADES_COUNT 5

const float PI = 3.141592653589793;

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

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight
{
    // in view space
    vec3 direction;

    vec3 color;
};

uniform bool directional_light_shadow_enabled = false;
uniform sampler2DArray directional_light_shadow_tex;
uniform mat4 light_space_matrices[CASCADES_COUNT];
uniform float cascades_plane_distances[CASCADES_COUNT];
uniform bool show_shadow_cascades = false;

uniform sampler2D in_albedo_tex;
uniform vec3 in_albedo_color = vec3(0.6);
uniform bool albedo_tex_enabled = false;

uniform sampler2D in_roughness_tex;
uniform float in_roughness = 0.6;
uniform bool roughness_tex_enabled = false;

uniform sampler2D in_ao_tex;
uniform bool ao_tex_enabled = false;

uniform sampler2D in_emissive_tex;
uniform bool emissive_tex_enabled = false;

uniform sampler2D in_normal_tex;
uniform bool normal_tex_enabled = false;

uniform samplerCube env_tex;
uniform samplerCube env_irradiance_tex;
uniform sampler2D brdf_lut_tex;

uniform int point_light_count = 0;
uniform PointLight point_lights[MAX_POINT_LIGHTS_COUNT];

uniform bool smooth_shadows = true;
uniform float light_size = 10.25f;
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
    const float uv_radius_max = 0.002f;
    float uv_radius = uv_radius_max;
    if (blocker_distance != -1.0)
    {
        float penumbra_width = (shadow_coords.z - blocker_distance) / blocker_distance;

        const float near = 0.01;
        float uv_radius = penumbra_width * uv_light_size * near / shadow_coords.z;
        uv_radius = min(uv_radius, uv_radius_max);
    }


    return directional_light_pcf(shadow_tex, cascade, shadow_coords, uv_radius);
}

int calc_cascade_index()
{
    float depth_value = abs(fs_in.position.z);

    int layer = -1;
    for (int i = 0; i < CASCADES_COUNT; ++i)
    {
        if (depth_value < cascades_plane_distances[i])
        {
            layer = i;
            break;
        }
    }

    if (layer == -1)
    {
        layer = CASCADES_COUNT;
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

vec3 calc_normal()
{
    if (normal_tex_enabled)
    {
        mat3 TBN = mat3(fs_in.tangent, fs_in.bitangent, fs_in.normal);
        vec3 normal = texture(in_normal_tex, fs_in.tex_coord).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(TBN * normal);
        return normal;
    }

    return fs_in.normal;
}

////////////////////////////////////////////////////////////////////////////////
// PBR based on:
// https://github.com/KhronosGroup/glTF-WebGL-PBR/blob/master/shaders/pbr-frag.glsl
////////////////////////////////////////////////////////////////////////////////

// Encapsulate the various inputs used by the various functions in the
// shading equation We store values in this struct to simplify the
// integration of alternative implementations of the shading terms,
// outlined in the Readme.MD Appendix.
struct PbrInfo
{
    // cos angle between normal and light direction
	float n_dot_l;

    // cos angle between normal and view direction
	float n_dot_v;

    // cos angle between normal and half vector
	float n_dot_h;

    // cos angle between light direction and half vector
	float l_dot_h;

    // cos angle between view direction and half vector
	float v_dot_h;

    // roughness value, as authored by the model creator (input to
    // shader)
	float perceptual_roughness;

    // full reflectance color (normal incidence angle)
	vec3 reflectance0;

    // reflectance color at grazing angle
	vec3 reflectance90;

    // roughness mapped to a more linear change in the roughness
    // (proposed by [2])
	float alpha_roughness;

    // color contribution from diffuse lighting
	vec3 diffuse_color;

    // color contribution from specular lighting
	vec3 specular_color;

    // normal at surface point
	vec3 n;

    // vector from surface point to camera
	vec3 v;
};

vec4 srgb_to_linear(vec4 srgb_in)
{
	vec3 lin_out = pow(srgb_in.xyz, vec3(2.2));
	return vec4(lin_out, srgb_in.a);
}

// Calculation of the lighting contribution from an optional Image
// Based Light source.  Precomputed Environment Maps are required
// uniform inputs and are computed as outlined in [1].  See our
// README.md on Environment Maps [3] for additional discussion.
vec3 calc_ibl_contribution(PbrInfo pbr_info, vec3 n, vec3 reflection)
{
	float mip_count = float(textureQueryLevels(env_tex));
	float lod = pbr_info.perceptual_roughness * mip_count;

	// retrieve a scale and bias to F0. See [1], Figure 3
	vec2 brdf_sample_point = vec2(pbr_info.n_dot_v,
                                  1.0 - pbr_info.perceptual_roughness);
	vec3 brdf = textureLod(brdf_lut_tex, brdf_sample_point, 0).rgb;
	vec3 cm = vec3(1.0, 1.0, 1.0);
	// HDR envmaps are already linear
	vec3 diffuseLight = texture(env_irradiance_tex, n.xyz * cm).rgb;
	vec3 specularLight = textureLod(env_tex, reflection.xyz * cm, lod).rgb;

	vec3 diffuse = diffuseLight * pbr_info.diffuse_color;
	vec3 specular = specularLight * (pbr_info.specular_color * brdf.x + brdf.y);

	return diffuse + specular;
}

// Disney Implementation of diffuse from Physically-Based Shading at
// Disney by Brent Burley. See Section 5.3.
// http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
vec3 diffuse_burley(PbrInfo pbr_info)
{
	float f90 = 2.0 * pbr_info.l_dot_h * pbr_info.l_dot_h * pbr_info.alpha_roughness - 0.5;

	return (pbr_info.diffuse_color / PI)
        * (1.0 + f90 * pow((1.0 - pbr_info.n_dot_l), 5.0))
        * (1.0 + f90 * pow((1.0 - pbr_info.n_dot_v), 5.0));
}

// The following equation models the Fresnel reflectance term of the
// spec equation (aka F()) Implementation of fresnel from [4],
// Equation 15
vec3 specular_reflection(PbrInfo pbr_info)
{
	return pbr_info.reflectance0
        + (pbr_info.reflectance90 - pbr_info.reflectance0)
        * pow(clamp(1.0 - pbr_info.v_dot_h, 0.0, 1.0), 5.0);
}

// This calculates the specular geometric attenuation (aka G()), where
// rougher material will reflect less light back to the viewer.  This
// implementation is based on [1] Equation 4, and we adopt their
// modifications to alpha_roughness as input as originally proposed in
// [2].
float geometric_occlusion(PbrInfo pbr_info)
{
	float n_dot_l = pbr_info.n_dot_l;
	float n_dot_v = pbr_info.n_dot_v;
	float r_sqr = pbr_info.alpha_roughness * pbr_info.alpha_roughness;

	float attenuation_l = 2.0 * n_dot_l
        / (n_dot_l + sqrt(r_sqr + (1.0 - r_sqr) * (n_dot_l * n_dot_l)));
	float attenuation_v = 2.0 * n_dot_v
        / (n_dot_v + sqrt(r_sqr + (1.0 - r_sqr) * (n_dot_v * n_dot_v)));
	return attenuation_l * attenuation_v;
}

// The following equation(s) model the distribution of microfacet
// normals across the area being drawn (aka D()) Implementation from
// "Average Irregularity Representation of a Roughened Surface for Ray
// Reflection" by T. S. Trowbridge, and K. P. Reitz Follows the
// distribution function recommended in the SIGGRAPH 2013 course notes
// from EPIC Games [1], Equation 3.
float microfacet_distribution(PbrInfo pbr_info)
{
	float roughness_sq = pbr_info.alpha_roughness * pbr_info.alpha_roughness;
	float f = (pbr_info.n_dot_h * roughness_sq - pbr_info.n_dot_h) * pbr_info.n_dot_h + 1.0;
	return roughness_sq / (PI * f * f);
}

vec3 calc_pbr_inputs_metallic_roughness(vec4 albedo,
                                        vec3 normal,
                                        vec4 mr_sample,
                                        out PbrInfo pbr_info)
{
	float perceptual_roughness = 1.0;
	float metallic = 1.0;

	// Roughness is stored in the 'g' channel, metallic is stored in
	// the 'b' channel.  This layout intentionally reserves the 'r'
	// channel for (optional) occlusion map data
	perceptual_roughness = mr_sample.g * perceptual_roughness;
	metallic = mr_sample.b * metallic;

	const float min_roughness = 0.04;

	perceptual_roughness = clamp(perceptual_roughness, min_roughness, 1.0);
	metallic = clamp(metallic, 0.0, 1.0);
	// Roughness is authored as perceptual roughness; as is
	// convention, convert to material roughness by squaring the
	// perceptual roughness [2].
	float alpha_roughness = perceptual_roughness * perceptual_roughness;

	// The albedo may be defined from a base texture or a flat color
	vec4 base_color = albedo;

	vec3 f0 = vec3(0.04);
	vec3 diffuse_color = base_color.rgb * (vec3(1.0) - f0);
	diffuse_color *= 1.0 - metallic;
	vec3 specular_color = mix(f0, base_color.rgb, metallic);

	// Compute reflectance.
	float reflectance = max(max(specular_color.r, specular_color.g), specular_color.b);

	// For typical incident reflectance range (between 4% to 100%) set
	// the grazing reflectance to 100% for typical fresnel effect.
	// For very low reflectance range on highly diffuse objects (below
	// 4%), incrementally reduce grazing reflecance to 0%.
	float reflectance90 = clamp(reflectance * 25.0, 0.0, 1.0);
	vec3 specular_environment_r0 = specular_color.rgb;
	vec3 specular_environment_r90 = vec3(1.0, 1.0, 1.0) * reflectance90;

	vec3 n = normalize(normal); // normal at surface point
	vec3 v = normalize(-fs_in.position); // Vector from surface point to camera
	vec3 reflection = -normalize(reflect(v, n));

	pbr_info.n_dot_v = clamp(abs(dot(n, v)), 0.001, 1.0);
	pbr_info.perceptual_roughness = perceptual_roughness;
	pbr_info.reflectance0 = specular_environment_r0;
	pbr_info.reflectance90 = specular_environment_r90;
	pbr_info.alpha_roughness = alpha_roughness;
	pbr_info.diffuse_color = diffuse_color;
	pbr_info.specular_color = specular_color;
	pbr_info.n = n;
	pbr_info.v = v;

	// Calculate lighting contribution from image based lighting source (IBL)
	vec3 color = calc_ibl_contribution(pbr_info, n, reflection);

	return color;
}

vec3 calc_pbr_point_light_contribution(inout PbrInfo pbr_info, PointLight point_light)
{
	vec3 n = pbr_info.n;
	vec3 v = pbr_info.v;
	// Vector from surface point to light
	vec3 l = normalize(point_light.position - fs_in.position);
    // Half vector between both l and v
	vec3 h = normalize(l + v);

    float n_dot_v = pbr_info.n_dot_v;
    float n_dot_l = clamp(dot(n, l), 0.001, 1.0);
    float n_dot_h = clamp(dot(n, h), 0.0, 1.0);
    float l_dot_h = clamp(dot(l, h), 0.0, 1.0);
    float v_dot_h = clamp(dot(v, h), 0.0, 1.0);

    pbr_info.n_dot_l = n_dot_l;
    pbr_info.n_dot_h = n_dot_h;
    pbr_info.l_dot_h = l_dot_h;
    pbr_info.v_dot_h = v_dot_h;

    vec3 F = specular_reflection(pbr_info);
    float G = geometric_occlusion(pbr_info);
    float D = microfacet_distribution(pbr_info);

    vec3 diffuse_contrib = (1.0 - F) * diffuse_burley(pbr_info);
    vec3 spec_contrib = F * G * D / (4.0 * n_dot_l * n_dot_v);

    float dist = length(point_light.position - fs_in.position);
    float attenuation = 1.0 / (point_light.constant + point_light.linear * dist
                               + point_light.quadratic * (dist * dist));
    vec3 radiance = point_light.color * attenuation;

    vec3 color = n_dot_l * radiance * (diffuse_contrib + spec_contrib);

    return color;
}

vec3 calc_pbr_light_contribution(inout PbrInfo pbr_info, vec3 lightDirection, vec3 lightColor)
{
    vec3 n = pbr_info.n;
    vec3 v = pbr_info.v;
    vec3 l = normalize(lightDirection);	// Vector from surface point to light
    vec3 h = normalize(l + v); // Half vector between both l and v

    float n_dot_v = pbr_info.n_dot_v;
    float n_dot_l = clamp(dot(n, l), 0.001, 1.0);
    float n_dot_h = clamp(dot(n, h), 0.0, 1.0);
    float l_dot_h = clamp(dot(l, h), 0.0, 1.0);
    float v_dot_h = clamp(dot(v, h), 0.0, 1.0);

    pbr_info.n_dot_l = n_dot_l;
    pbr_info.n_dot_h = n_dot_h;
    pbr_info.l_dot_h = l_dot_h;
    pbr_info.v_dot_h = v_dot_h;

    // Calculate the shading terms for the microfacet specular shading model
    vec3 F = specular_reflection(pbr_info);
    float G = geometric_occlusion(pbr_info);
    float D = microfacet_distribution(pbr_info);

    // Calculation of analytical lighting contribution
    vec3 diffuse_contrib = (1.0 - F) * diffuse_burley(pbr_info);
    vec3 spec_contrib = F * G * D / (4.0 * n_dot_l * n_dot_v);
    // Obtain final intensity as reflectance (BRDF) scaled by the energy of the light (cosine law)

    if (directional_light_shadow_enabled)
    {
        float shadow = calc_directional_light_shadow();
        diffuse_contrib *= shadow;
        spec_contrib *= shadow;
    }

    vec3 color = n_dot_l * lightColor * (diffuse_contrib + spec_contrib);

    return color;
}

vec4 calc_albedo_color()
{
    if (albedo_tex_enabled)
    {
        vec4 albedo_texture = texture(in_albedo_tex, fs_in.tex_coord);
        if (albedo_texture.a <= 0.01f)
        {
            discard;
        }
        return albedo_texture;
    }
    return vec4(in_albedo_color, 1.0);
}

vec4 calc_roughness()
{
    if (roughness_tex_enabled)
    {
        vec4 roughness_texture = texture(in_roughness_tex, fs_in.tex_coord);
        return roughness_texture;
    }
    return vec4(in_roughness);
}

vec4 calc_emissive()
{
    if (emissive_tex_enabled)
    {
        vec4 emissive_texture = texture(in_emissive_tex, fs_in.tex_coord);
        return emissive_texture;
    }
    return vec4(0.0);
}

vec4 calc_ao()
{
    if (ao_tex_enabled)
    {
        vec4 ao_texture = texture(in_ao_tex, fs_in.tex_coord);
        return ao_texture;
    }
    return vec4(0.0);
}

void main()
{
    vec3 normal = calc_normal();
    vec4 albedo = calc_albedo_color();
    vec4 ao = calc_ao();
    vec4 roughness = calc_roughness();
    vec4 emissive = calc_emissive();

    PbrInfo pbr_info;
    emissive.rgb = srgb_to_linear(emissive).rgb;
    // image based lightning
    vec3 color = calc_pbr_inputs_metallic_roughness(albedo, normal, roughness, pbr_info);
    // directionl light source
    if (directional_light_enabled)
    {
        color += calc_pbr_light_contribution(pbr_info, -directional_light.direction, directional_light.color);
    }
    // point lights
    for (int i = 0; i < point_light_count; ++i)
    {
        color += calc_pbr_point_light_contribution(pbr_info, point_lights[i]);
    }
    // ambient occlusion
    color = color * (ao.r < 0.01 ? 1.0 : ao.r);
    // emissive
    if (emissive_tex_enabled)
    {
        // color = pow(emissive.rgb + color, vec3(1.0 / 2.2));
        color += emissive.rgb;
    }

    // for (int i = 0; i < point_light_count; ++i)
    // {
    //   color += blinn_phong_point_light(point_lights[i],
    //                                    ambient_color,
    //                                    diffuse_color,
    //                                    specular_color,
    //                                    normal);
    // }
    // for (int i = 0; i < spot_light_count; ++i)
    // {
    //   color += blinn_phong_spot_light(spot_lights[i],
    //                                   ambient_color,
    //                                   diffuse_color,
    //                                   specular_color,
    //                                   normal);
    // }
    // if (directional_light_enabled)
    // {
    //   color += blinn_phong_directional_light(ambient_color,
    //                                          diffuse_color,
    //                                          specular_color,
    //                                          normal);
    // }

    if (show_shadow_cascades)
    {
        vec3 cascade_color = vec3(1.0f);
        int cascade_index = calc_cascade_index();
        if (cascade_index == 0)
        {
            cascade_color = vec3(1.0f, 0.0f, 0.0f);
        }
        else if (cascade_index == 1)
        {
            cascade_color = vec3(0.0f, 1.0f, 0.0f);
        }
        else if (cascade_index == 2)
        {
            cascade_color = vec3(0.0f, 0.0f, 1.0f);
        }
        else if (cascade_index == 3)
        {
            cascade_color = vec3(0.0f, 1.0f, 1.0f);
        }
        else if (cascade_index == 4)
        {
            cascade_color = vec3(1.0f, 1.0f, 0.0f);
        }
        color *= cascade_color;
    }

    out_color = vec4(color, 1.0);
}
