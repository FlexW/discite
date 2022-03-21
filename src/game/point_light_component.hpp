#pragma once

#include "math.hpp"

namespace dc
{

struct PointLightComponent
{
  glm::vec3 color_{1.0f};
  float     multiplier_{1.0f};

  float radius_{10.0f};
  float falloff_{0.2f};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
