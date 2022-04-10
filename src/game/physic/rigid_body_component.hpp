#pragma once

#include "math.hpp"
#include "rigid_body.hpp"

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

  RigidBody *physic_actor_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
