#pragma once

#include "math.hpp"

struct DirectionalLightComponent
{
  glm::vec3 ambient_color_{10.0f};
  glm::vec3 diffuse_color_{50.0f};
  glm::vec3 specular_color_{60.0f};
};
