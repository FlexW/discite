#include "point_light.hpp"

namespace dc
{

void PointLight::set_position(const glm::vec3 &value) { position_ = value; }

glm::vec3 PointLight::position() const { return position_; }

void PointLight::set_color(const glm::vec3 &value) { color_ = value; }

glm::vec3 PointLight::color() const { return color_; }

void PointLight::set_constant(float value) { constant_ = value; }

float PointLight::constant() const { return constant_; }

void PointLight::set_linear(float value) { linear_ = value; }

float PointLight::linear() const { return linear_; }

void PointLight::set_quadratic(float value) { quadratic_ = value; }

float PointLight::quadratic() const { return quadratic_; }

} // namespace dc
