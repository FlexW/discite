#pragma once

#include "math.hpp"

struct PointLightComponent
{
  glm::vec3 color_{1.0f};

  float constant_{1.0f};
  float linear_{0.9f};
  float quadratic_{1.8f};

  void save(FILE *file) const;
  void read(FILE *file);
};
