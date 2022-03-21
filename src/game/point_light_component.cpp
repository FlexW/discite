#include "point_light_component.hpp"
#include "serialization.hpp"

namespace dc
{

void PointLightComponent::save(FILE *file) const
{
  write_value(file, color_);
  write_value(file, multiplier_);
  write_value(file, radius_);
  write_value(file, falloff_);
}

void PointLightComponent::read(FILE *file)
{
  read_value(file, color_);
  read_value(file, multiplier_);
  read_value(file, radius_);
  read_value(file, falloff_);
}

} // namespace dc
