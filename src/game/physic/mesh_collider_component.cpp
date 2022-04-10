#include "mesh_collider_component.hpp"
#include "serialization.hpp"

namespace dc
{

void MeshColliderComponent::save(FILE *file) const
{
  write_value(file, is_trigger_);
  write_value(file, is_convex_);
  write_value(file, physic_material_);
}

void MeshColliderComponent::read(FILE *file)
{
  read_value(file, is_trigger_);
  read_value(file, is_convex_);
  read_value(file, physic_material_);
}

} // namespace dc
