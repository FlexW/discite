#version 460 core

#define MAX_POINT_LIGHTS_COUNT 5
#define MAX_SPOT_LIGHTS_COUNT 5

in VS_OUT
{
  vec3 position;
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

uniform sampler2D in_ambient_tex;
uniform vec3 in_ambient_color = vec3(0.6);
uniform bool ambient_tex_enabled = false;

uniform sampler2D in_diffuse_tex;
uniform vec3 in_diffuse_color = vec3(0.6);
uniform bool diffuse_tex_enabled = false;

uniform sampler2D in_specular_tex;
uniform vec3 in_specular_color = vec3(0.6);
uniform bool specular_tex_enabled = false;

uniform float specular_power = 200.0f;

uniform sampler2D in_normal_tex;
uniform bool normal_tex_enabled = false;

uniform int point_light_count;
uniform PointLight point_lights[MAX_POINT_LIGHTS_COUNT];
uniform int spot_light_count;
uniform SpotLight spot_lights[MAX_SPOT_LIGHTS_COUNT];

uniform bool directional_light_enabled;
uniform DirectionalLight directional_light;

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
        specular = specular_color * pow(max(dot(N, H), 0.0), specular_power) * spot_light.specular_color;
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
        specular = specular_color * pow(max(dot(N, H), 0.0), specular_power) * directional_light.specular_color;
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

vec3 calc_ambient_color()
{
    if (ambient_tex_enabled)
    {
        vec4 ambient_texture = texture(in_ambient_tex, fs_in.tex_coord).rgba;
        if (ambient_texture.a <= 0.01f)
        {
            discard;
        }
        return ambient_texture.rgb;
    }
    else if (diffuse_tex_enabled)
    {
        return calc_diffuse_color();
    }
    return in_ambient_color;
}

vec3 calc_specular_color()
{
    if (specular_tex_enabled)
    {
        vec4 specular_texture = texture(in_specular_tex, fs_in.tex_coord).rrra;
        return specular_texture.rgb;
    }
    else if (diffuse_tex_enabled)
    {
        return calc_diffuse_color();
    }
    return in_specular_color;
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
  vec3 ambient_color = calc_ambient_color();
  vec3 diffuse_color = calc_diffuse_color();
  vec3 specular_color = calc_specular_color();

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
  out_color = vec4(color, 1.0);
}
