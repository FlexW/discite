#pragma once

#include "character_controller.hpp"
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

  BoxCollider         *box_collider_{};
  CharacterController *character_controller_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
