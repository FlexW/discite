#pragma once

#include "math.hpp"
#include "physic_actor.hpp"

namespace dc
{

struct RigidBodyComponent
{
  RigidBodyType body_type_{RigidBodyType::Dynamic};

  float mass_{1.0f};
  float linear_drag_{0.01f};
  float angular_drag_{0.05f};

  bool is_kinematic_{false};

  bool is_gravity_disabled_{false};

  PhysicActor *physic_actor_{};
};

} // namespace dc
