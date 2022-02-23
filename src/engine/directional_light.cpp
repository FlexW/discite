#include "directional_light.hpp"
#include "glm/geometric.hpp"

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
