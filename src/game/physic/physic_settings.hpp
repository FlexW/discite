#pragma once

#include "physic/physic_types.hpp"

namespace dc
{

struct PhysicsSettings
{
  float          fixed_timestep             = 1.0f / 60.0f;
  glm::vec3      gravity                    = {0.0f, -9.81f, 0.0f};
  BroadphaseType broadphase_algorithm       = BroadphaseType::AutomaticBoxPrune;
  glm::vec3      world_bounds_min           = glm::vec3{-100.0f};
  glm::vec3      world_bounds_max           = glm::vec3{100.0f};
  uint32_t       world_bounds_subdivisions  = 2;
  FrictionType   friction_model             = FrictionType::Patch;
  std::uint32_t  solver_iterations          = 8;
  std::uint32_t  solver_velocity_iterations = 2;
};

PhysicsSettings default_physic_settings();

} // namespace dc
