#pragma once

#include "camera.hpp"

struct CameraComponent
{
  bool   active_{true};
  Camera camera_;

  CameraComponent() = default;
  CameraComponent(float near_plane, float far_plane, float aspect_ratio);

  void save(FILE *file) const;
  void read(FILE *file);
};
