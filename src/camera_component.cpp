#include "camera_component.hpp"

CameraComponent::CameraComponent(float near_plane,
                                 float far_plane,
                                 float aspect_ratio)
{
  camera_.set_near_plane(near_plane);
  camera_.set_far_plane(far_plane);
  camera_.set_aspect_ratio(aspect_ratio);
}
