#include "sphere_collider_component.hpp"
#include "serialization.hpp"

namespace dc
{

void SphereColliderComponent::save(FILE *file) const
{
  write_value(file, radius_);
  write_value(file, offset_);
  write_value(file, is_trigger_);
  write_value(file, physic_material_);
}

void SphereColliderComponent::read(FILE *file)
{
  read_value(file, radius_);
  read_value(file, offset_);
  read_value(file, is_trigger_);
  read_value(file, physic_material_);
}

} // namespace dc
