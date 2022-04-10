#pragma once

#include "character_controller.hpp"
#include "math.hpp"
#include "physic_material.hpp"

namespace dc
{

class CapsuleCollider;

struct CapsuleColliderComponent
{
  float     radius_{0.5f};
  float     height_{1.0f};
  glm::vec3 offset_{0.0f};

  bool is_trigger_{false};

  PhysicMaterial physic_material_{};

  CapsuleCollider     *capsule_collider_{};
  CharacterController *character_controller_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
