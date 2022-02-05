#version 460 core

#define MAX_POINT_LIGHTS_COUNT 5
#define MAX_SPOT_LIGHTS_COUNT 5

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
  // In view space
  vec3 position;

  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;

  float constant;
  float linear;
  float quadratic;
};

struct DirectionalLight
{
  vec3 direction;
  vec3 direction_ws;

  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
};

struct SpotLight
{
  // In view space
  vec3 position;
  vec3 direction;

  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;

  float constant;
  float linear;
  float quadratic;

  float cut_off;
  float outer_cut_off;
};

#define CASCADES_COUNT 5
uniform bool directional_light_shadow_enabled = false;
uniform sampler2DArray directional_light_shadow_tex;
uniform float far_plane;
uniform mat4 light_space_matrices[CASCADES_COUNT];
uniform float cascades_plane_distances[CASCADES_COUNT];
uniform bool show_shadow_cascades = false;

uniform sampler2D in_diffuse_tex;
uniform vec3 in_diffuse_color = vec3(0.6);
uniform bool diffuse_tex_enabled = false;

uniform float specular_power = 200.0f;

uniform sampler2D in_normal_tex;
uniform bool normal_tex_enabled = false;

uniform int point_light_count = 0;
uniform PointLight point_lights[MAX_POINT_LIGHTS_COUNT];
uniform int spot_light_count = 0;
uniform SpotLight spot_lights[MAX_SPOT_LIGHTS_COUNT];

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

    if (layer == CASCADES_COUNT)
    {
        bias * 1.0 / (far_plane * 0.5f);
    }
    else
    {
        bias *= 1.0 / (cascades_plane_distances[layer] * 0.5f);
    }
    return bias;
}

float search_region_radius_uv(float z_world)
{
    const float light_z_near = 0.0;
    const float light_radius_uv = 0.05;
    return light_radius_uv * (z_world - light_z_near) / z_world;
}

#define POISSON_DISTRIBUTION_COUNT 64

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

vec3 blinn_phong_point_light(PointLight point_light,
                             vec3 ambient_color,
                             vec3 diffuse_color,
                             vec3 specular_color,
                             vec3 normal)
{
    vec3 V = normalize(-fs_in.position);
    vec3 N = normal;
    vec3 L = normalize(point_light.position - fs_in.position);
    vec3 ambient = ambient_color * point_light.ambient_color;
    float N_dot_L = max(dot(N, L), 0.0);
    vec3 diffuse = N_dot_L * diffuse_color * point_light.diffuse_color;
    vec3 specular = vec3(0.0);
    if (N_dot_L > 0.0)
    {
        vec3 H = normalize(L + V);
        specular = specular_color * pow(max(dot(N, H), 0.0), specular_power) * point_light.specular_color;
    }

    float dist = length(point_light.position - fs_in.position);
    float attenuation = 1.0 / (point_light.constant + point_light.linear * dist
        + point_light.quadratic * (dist * dist));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 blinn_phong_spot_light(SpotLight spot_light,
                            vec3 ambient_color,
                            vec3 diffuse_color,
                            vec3 specular_color,
                            vec3 normal)
{
    vec3 V = normalize(-fs_in.position);
    vec3 N = normal;
    vec3 L = normalize(spot_light.position - fs_in.position);
    vec3 ambient = ambient_color * spot_light.ambient_color;
    float N_dot_L = max(dot(N, L), 0.0);
    vec3 diffuse = N_dot_L * diffuse_color * spot_light.diffuse_color;
    vec3 specular = vec3(0.0);
    if (N_dot_L > 0.0)
    {
        vec3 H = normalize(L + V);
        specular = specular_color * pow(max(dot(N, H), 0.0), specular_power)
            * spot_light.specular_color;
    }

    float theta = dot(L, normalize(-spot_light.direction));
    float epsilon = (spot_light.cut_off - spot_light.outer_cut_off);
    float intensity = clamp((theta - spot_light.outer_cut_off) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    float dist = length(spot_light.position - fs_in.position);
    float attenuation = 1.0 / (spot_light.constant + spot_light.linear * dist
        + spot_light.quadratic * (dist * dist));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return ambient + diffuse + specular;
}

vec3 blinn_phong_directional_light(vec3 ambient_color,
                                   vec3 diffuse_color,
                                   vec3 specular_color,
                                   vec3 normal)
{
    vec3 N = normal;
    vec3 V = normalize(-fs_in.position);
    vec3 L = normalize(-directional_light.direction);

    // Compute lightning
    vec3 ambient = ambient_color * directional_light.ambient_color;
    float N_dot_L = max(dot(N, L), 0.0);
    vec3 diffuse = N_dot_L * diffuse_color * directional_light.diffuse_color;
    vec3 specular = vec3(0.0);
    if (N_dot_L > 0.0)
    {
        vec3 H = normalize(L + V);
        specular = specular_color * pow(max(dot(N, H), 0.0), specular_power)
            * directional_light.specular_color;
    }

    if (directional_light_shadow_enabled)
    {
        float shadow = calc_directional_light_shadow();
        diffuse *= shadow;
        specular *= shadow;
    }

    return ambient + diffuse + specular;
}

vec3 calc_diffuse_color()
{
    if (diffuse_tex_enabled)
    {
        vec4 diffuse_texture = texture(in_diffuse_tex, fs_in.tex_coord).rgba;
        if (diffuse_texture.a <= 0.01f)
        {
            discard;
        }
        return diffuse_texture.rgb;
    }
    return in_diffuse_color;
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

void main()
{
  vec3 normal = calc_normal();
  vec3 diffuse_color = calc_diffuse_color();
  vec3 ambient_color = diffuse_color;
  vec3 specular_color = diffuse_color;

  vec3 color = vec3(0.0f);
  for (int i = 0; i < point_light_count; ++i)
  {
    color += blinn_phong_point_light(point_lights[i],
                                     ambient_color,
                                     diffuse_color,
                                     specular_color,
                                     normal);
  }
  for (int i = 0; i < spot_light_count; ++i)
  {
    color += blinn_phong_spot_light(spot_lights[i],
                                    ambient_color,
                                    diffuse_color,
                                    specular_color,
                                    normal);
  }
  if (directional_light_enabled)
  {
    color += blinn_phong_directional_light(ambient_color,
                                           diffuse_color,
                                           specular_color,
                                           normal);
  }

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
