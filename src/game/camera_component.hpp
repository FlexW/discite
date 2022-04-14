#pragma once

#include "camera.hpp"

#include <cstdint>
#include <utility>

namespace dc
{

struct CameraComponent
{
  bool primary_{true};

  ProjectionType projection_type_{ProjectionType::Perspective};

  float fov_degree_{45.0f};
  float perspective_near_{0.1f};
  float perspective_far_{1000.0f};

  float orthographic_near_{-1.0f};
  float orthographic_far_{1.0f};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
