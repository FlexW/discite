#pragma once

#include "math.hpp"

namespace dc
{

class DirectionalLight
{
public:
  void      set_direction(const glm::vec3 &value);
  glm::vec3 direction() const;

  void      set_color(const glm::vec3 &value);
  glm::vec3 color() const;

  void set_multiplier(float value);
  float multiplier() const;

private:
  glm::vec3 direction_{glm::normalize(glm::vec3{0.1f, -1.0f, -0.1f})};
  float     multiplier_{1.0f};
  glm::vec3 color_{1.0f};
};

} // namespace dc
