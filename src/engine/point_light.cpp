#include "point_light.hpp"

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

} // namespace dc
