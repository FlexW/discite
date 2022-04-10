#include "character_controller_component.hpp"
#include "serialization.hpp"

namespace dc
{

void CharacterControllerComponent::save(FILE *file) const
{
  write_value(file, slope_limit_degree_);
  write_value(file, step_offset_);
  write_value(file, disable_gravity_);
}

void CharacterControllerComponent::read(FILE *file)
{
  read_value(file, slope_limit_degree_);
  read_value(file, step_offset_);
  read_value(file, disable_gravity_);
}

} // namespace dc
