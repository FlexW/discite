#pragma once

#include "math.hpp"
#include "physic_material.hpp"

namespace dc
{

class SphereCollider;

struct SphereColliderComponent
{
  float     radius_{0.5f};
  glm::vec3 offset_{0.0f};

  bool is_trigger_{false};

  PhysicMaterial physic_material_{};

  SphereCollider *sphere_collider_{};
};

} // namespace dc
