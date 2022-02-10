#pragma once

#include "engine/camera.hpp"

struct CameraComponent
{
  bool   active_{true};
  Camera camera_;

  CameraComponent(float near_plane, float far_plane, float aspect_ratio);
};
