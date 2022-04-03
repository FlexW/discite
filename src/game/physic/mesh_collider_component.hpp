#pragma once

#include "physic_material.hpp"

namespace dc
{

struct MeshColliderComponent
{
  bool is_trigger_{false};
  bool is_convex_{false};

  PhysicMaterial physic_material_{};
};

} // namespace dc
