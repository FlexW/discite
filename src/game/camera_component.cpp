#include "camera_component.hpp"
#include "camera.hpp"
#include "serialization.hpp"

namespace dc
{

void CameraComponent::save(FILE *file) const
{
  write_value(file, primary_);
  write_value(file, projection_type_);
  write_value(file, fov_degree_);
  write_value(file, perspective_near_);
  write_value(file, perspective_far_);
  write_value(file, orthographic_near_);
  write_value(file, orthographic_far_);
}

void CameraComponent::read(FILE *file)
{
  read_value(file, primary_);
  read_value(file, projection_type_);
  read_value(file, fov_degree_);
  read_value(file, perspective_near_);
  read_value(file, perspective_far_);
  read_value(file, orthographic_near_);
  read_value(file, orthographic_far_);
}

} // namespace dc
