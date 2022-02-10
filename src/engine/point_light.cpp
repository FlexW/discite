#include "point_light.hpp"

void PointLight::set_position(const glm::vec3 &value) { position_ = value; }

glm::vec3 PointLight::position() const { return position_; }

void PointLight::set_ambient_color(const glm::vec3 &value)
{
  ambient_color_ = value;
}

glm::vec3 PointLight::ambient_color() const { return ambient_color_; }

void PointLight::set_diffuse_color(const glm::vec3 &value)
{
  diffuse_color_ = value;
}

glm::vec3 PointLight::diffuse_color() const { return diffuse_color_; }

void PointLight::set_specular_color(const glm::vec3 &value)
{
  specular_color_ = value;
}

glm::vec3 PointLight::specular_color() const { return specular_color_; }

void PointLight::set_constant(float value) { constant_ = value; }

float PointLight::constant() const { return constant_; }

void PointLight::set_linear(float value) { linear_ = value; }

float PointLight::linear() const { return linear_; }

void PointLight::set_quadratic(float value) { quadratic_ = value; }

float PointLight::quadratic() const { return quadratic_; }
