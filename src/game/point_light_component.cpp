#include "point_light_component.hpp"
#include "point_light.hpp"
#include "serialization.hpp"

namespace dc
{

void PointLightComponent::save(FILE *file) const
{
  write_value(file, color_);
  write_value(file, multiplier_);
  write_value(file, radius_);
  write_value(file, falloff_);
  write_value(file, cast_shadow_);
}

void PointLightComponent::read(FILE *file)
{
  read_value(file, color_);
  read_value(file, multiplier_);
  read_value(file, radius_);
  read_value(file, falloff_);
  bool is_cast_shadow{false};
  read_value(file, is_cast_shadow);
  set_cast_shadow(is_cast_shadow);
}

void PointLightComponent::set_cast_shadow(bool value)
{
  if (value == cast_shadow_)
  {
    // nothing to do
    return;
  }
  cast_shadow_ = value;
  if (value)
  {
    GlCubeTextureConfig shadow_tex_config{};
    shadow_tex_config.width_            = PointLight::shadow_map_size;
    shadow_tex_config.height_           = PointLight::shadow_map_size;
    shadow_tex_config.format            = GL_DEPTH_COMPONENT;
    shadow_tex_config.sized_format      = GL_DEPTH_COMPONENT32F;
    shadow_tex_config.wrap_s_           = GL_CLAMP_TO_EDGE;
    shadow_tex_config.wrap_t_           = GL_CLAMP_TO_EDGE;
    shadow_tex_config.wrap_r_           = GL_CLAMP_TO_EDGE;
    shadow_tex_config.min_filter_       = GL_NEAREST;
    shadow_tex_config.mag_filter_       = GL_NEAREST;
    shadow_tex_config.generate_mipmaps_ = false;
    shadow_tex_config.type_             = GL_FLOAT;
    shadow_tex_ = std::make_shared<GlCubeTexture>(shadow_tex_config);
  }
  else
  {
    shadow_tex_ = nullptr;
  }
}

bool PointLightComponent::cast_shadow() const { return cast_shadow_; }

std::shared_ptr<GlCubeTexture> PointLightComponent::shadow_tex() const
{
  return shadow_tex_;
}

} // namespace dc
