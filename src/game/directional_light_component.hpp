#pragma once

#include "math.hpp"

namespace dc
{

struct DirectionalLightComponent
{
  glm::vec3 color_{1.0f};
  float     multiplier_{1.0f};
  bool      cast_shadow_{true};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
