#include "directional_light.hpp"
#include "glm/geometric.hpp"

namespace dc
{

void DirectionalLight::set_direction(const glm::vec3 &value)
{
  direction_ = value;
}

glm::vec3 DirectionalLight::direction() const
{
  return glm::normalize(direction_);
}

void DirectionalLight::set_color(const glm::vec3 &value) { color_ = value; }

glm::vec3 DirectionalLight::color() const { return color_; }

void DirectionalLight::set_multiplier(float value) { multiplier_ = value; }

float DirectionalLight::multiplier() const { return multiplier_; }

} // namespace dc
