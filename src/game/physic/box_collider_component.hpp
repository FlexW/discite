#pragma once

#include "math.hpp"
#include "physic_material.hpp"

namespace dc
{

class BoxCollider;

struct BoxColliderComponent
{
  glm::vec3 size_{1.0f};
  glm::vec3 offset_{0.0f};

  bool is_trigger_{false};

  PhysicMaterial physic_material_{};

  BoxCollider *box_collider_{};
};

} // namespace dc
