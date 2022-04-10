#include "capsule_collider_component.hpp"
#include "serialization.hpp"

namespace dc
{

void CapsuleColliderComponent::save(FILE *file) const
{
  write_value(file, radius_);
  write_value(file, height_);
  write_value(file, offset_);
  write_value(file, physic_material_);
}

void CapsuleColliderComponent::read(FILE *file)
{
  read_value(file, radius_);
  read_value(file, height_);
  read_value(file, offset_);
  read_value(file, physic_material_);
}

} // namespace dc
