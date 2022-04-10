#pragma once

#include "mesh_collider.hpp"
#include "physic_material.hpp"

namespace dc
{

struct MeshColliderComponent
{
  bool is_trigger_{false};
  bool is_convex_{false};

  PhysicMaterial physic_material_{};

  MeshCollider *mesh_collider_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
