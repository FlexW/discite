#pragma once

#include "engine/math.hpp"

struct PointLightComponent
{
  glm::vec3 ambient_color_{5.0f};
  glm::vec3 diffuse_color_{10.0f};
  glm::vec3 specular_color_{20.0f};

  float constant_{1.0f};
  float linear_{0.9f};
  float quadratic_{1.8f};
};
