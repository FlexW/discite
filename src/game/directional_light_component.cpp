#include "directional_light_component.hpp"
#include "serialization.hpp"

namespace dc
{

void DirectionalLightComponent::save(FILE *file) const
{
  write_value(file, color_);
  write_value(file, multiplier_);
}

void DirectionalLightComponent::read(FILE *file)
{
  read_value(file, color_);
  read_value(file, multiplier_);
}

} // namespace dc
