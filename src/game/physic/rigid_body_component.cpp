#include "rigid_body_component.hpp"
#include "rigid_body.hpp"
#include "serialization.hpp"

#include <cstdint>

namespace dc
{

void RigidBodyComponent::save(FILE *file) const
{

  if (body_type_ == RigidBodyType::Dynamic)
  {
    write_value(file, static_cast<std::uint8_t>(0));
  }
  else if (body_type_ == RigidBodyType::Static)
  {
    write_value(file, static_cast<std::uint8_t>(1));
  }
  else
  {
    DC_FAIL("No such rigid body type");
  }

  write_value(file, mass_);
  write_value(file, linear_drag_);
  write_value(file, angular_drag_);
  write_value(file, is_kinematic_);
  write_value(file, is_gravity_disabled_);
}

void RigidBodyComponent::read(FILE *file)
{

  std::uint8_t rigid_body_type{0};
  read_value(file, rigid_body_type);
  if (rigid_body_type == 0)
  {
    body_type_ = RigidBodyType::Dynamic;
  }
  else if (rigid_body_type == 1)
  {
    body_type_ = RigidBodyType::Static;
  }
  else
  {
    DC_FAIL("No such rigid body type");
  }

  read_value(file, mass_);
  read_value(file, linear_drag_);
  read_value(file, angular_drag_);
  read_value(file, is_kinematic_);
  read_value(file, is_gravity_disabled_);
}

} // namespace dc
