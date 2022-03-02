#pragma once

#include "math.hpp"

struct DirectionalLightComponent
{
  glm::vec3 color_{1.0f};

  void save(FILE *file) const;
  void read(FILE *file);
};
