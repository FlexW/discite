#include "point_light.hpp"
#include "assert.hpp"

namespace dc
{

void PointLight::set_position(const glm::vec3 &value) { position_ = value; }

glm::vec3 PointLight::position() const { return position_; }

void PointLight::set_color(const glm::vec3 &value) { color_ = value; }

glm::vec3 PointLight::color() const { return color_; }

void PointLight::set_multiplier(float value) { multiplier_ = value; }

float PointLight::multiplier() const { return multiplier_; }

void PointLight::set_radius(float value) { radius_ = value; }

float PointLight::radius() const { return radius_; }

void PointLight::set_falloff(float value) { falloff_ = value; }

float PointLight::falloff() const { return falloff_; }

void PointLight::set_cast_shadow(bool value)
{
  if (value)
  {
    DC_ASSERT(shadow_tex_, "Shadow tex needs to be set");
  }

  cast_shadow_ = value;
}

bool PointLight::cast_shadow() const { return cast_shadow_; }

void PointLight::set_shadow_tex(std::shared_ptr<GlCubeTexture> value)
{
  shadow_tex_ = value;
}

std::shared_ptr<GlCubeTexture> PointLight::shadow_tex() const
{
  return shadow_tex_;
}

} // namespace dc
