#include "point_light_component.hpp"
#include "serialization.hpp"

namespace dc
{

void PointLightComponent::save(FILE *file) const
{
  write_value(file, color_);
  write_value(file, constant_);
  write_value(file, linear_);
  write_value(file, quadratic_);
}

void PointLightComponent::read(FILE *file)
{
  read_value(file, color_);
  read_value(file, constant_);
  read_value(file, linear_);
  read_value(file, quadratic_);
}

} // namespace dc
