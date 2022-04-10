#include "box_collider_component.hpp"
#include "serialization.hpp"

namespace dc
{

void BoxColliderComponent::save(FILE *file) const
{
  write_value(file, size_);
  write_value(file, offset_);
  write_value(file, is_trigger_);
  write_value(file, physic_material_);
}

void BoxColliderComponent::read(FILE *file)
{
  read_value(file, size_);
  read_value(file, offset_);
  read_value(file, is_trigger_);
  read_value(file, physic_material_);
}

} // namespace dc
