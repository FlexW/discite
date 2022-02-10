#include "directional_light.hpp"

DirectionalLight::DirectionalLight()
{
  set_direction(-glm::vec3{20.0f, 50.0f, 20.0f});
}

void DirectionalLight::set_direction(const glm::vec3 &value)
{
  direction_ = glm::normalize(value);
}

glm::vec3 DirectionalLight::direction() const { return direction_; }

void DirectionalLight::set_ambient_color(const glm::vec3 &value)
{
  ambient_color_ = value;
}

glm::vec3 DirectionalLight::ambient_color() const { return ambient_color_; }

void DirectionalLight::set_diffuse_color(const glm::vec3 &value)
{
  diffuse_color_ = value;
}

glm::vec3 DirectionalLight::diffuse_color() const { return diffuse_color_; }

void DirectionalLight::set_specular_color(const glm::vec3 &value)
{
  specular_color_ = value;
}

glm::vec3 DirectionalLight::specular_color() const { return specular_color_; }
