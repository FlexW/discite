#include "camera.hpp"
#include "serialization.hpp"

namespace dc
{

Camera::Camera()
{
  update_camera_vectors();
  recalculate_projection_matrix();
}

glm::mat4 Camera::view_matrix() const
{
  return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::update_movement(float delta_time)
{
  const auto front = free_fly_ ? front_ : front_movement_;

  glm::vec3 movement{0.0f};
  if (move_forward_)
  {
    movement += front;
  }
  if (move_backward_)
  {
    movement -= front;
  }
  if (move_left_)
  {
    movement -= right_;
  }
  if (move_right_)
  {
    movement += right_;
  }

  if (is_enable_acceleration_)
  {
    if (movement == glm::vec3{0.0f})
    {
      // decelerate naturally according to the damping value
      movement_speed_ -=
          movement_speed_ * std::min((1.0f / damping_) * delta_time, 1.0f);
    }
    else
    {
      // acceleration
      movement_speed_ += movement * acceleration_ * delta_time;
      if (glm::length(movement_speed_) > max_movement_speed_)
      {
        movement_speed_ = glm::normalize(movement_speed_) * max_movement_speed_;
      }
    }
  }
  else
  {
    if (movement == glm::vec3{0.0f})
    {
      movement_speed_ = glm::vec3{0.0f};
    }
    else
    {
      movement_speed_ =
          glm::normalize(movement * delta_time) * max_movement_speed_;
    }
  }

  position_ += movement_speed_ * delta_time;
}

void Camera::update_rotation(float xoffset, float yoffset, bool constrain_pitch)
{
  xoffset *= mouse_sensitivity_;
  yoffset *= mouse_sensitivity_;

  yaw_ += xoffset;
  pitch_ += yoffset;

  // Make sure that when pitch is out of bounds, screen doesn't get flipped
  if (constrain_pitch)
  {
    if (pitch_ > 89.0f)
    {
      pitch_ = 89.0f;
    }
    if (pitch_ < -89.0f)
    {
      pitch_ = -89.0f;
    }
  }

  // update Front, Right and Up Vectors using the updated Euler angles
  update_camera_vectors();
}

void Camera::process_scroll(float yoffset)
{
  zoom_ -= yoffset;

  if (zoom_ < 1.0f)
  {
    zoom_ = 1.0f;
  }
  if (zoom_ > 45.0f)
  {
    zoom_ = 45.0f;
  }
}

void Camera::update_camera_vectors()
{
  // Calculate the new Front vector
  glm::vec3 f;
  f.x    = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  f.y    = sin(glm::radians(pitch_));
  f.z    = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
  front_ = glm::normalize(f);

  f.x             = cos(glm::radians(yaw_)) * cos(glm::radians(0.0f));
  f.y             = sin(glm::radians(0.0f));
  f.z             = sin(glm::radians(yaw_)) * cos(glm::radians(0.0f));
  front_movement_ = glm::normalize(f);

  // Also re-calculate the Right and Up vector
  // normalize the vectors, because their length
  // gets closer to 0 the more you look up or down
  // which results in slower movement.
  right_ = glm::normalize(glm::cross(front_, world_up_));

  up_ = glm::normalize(glm::cross(right_, front_));
}

void Camera::set_position(const glm::vec3 &value) { position_ = value; }

void Camera::set_move_forward(bool value) { move_forward_ = value; }

void Camera::set_move_backward(bool value) { move_backward_ = value; }

void Camera::set_move_left(bool value) { move_left_ = value; }

void Camera::set_move_right(bool value) { move_right_ = value; }

void Camera::clear_movement()
{
  move_forward_  = false;
  move_backward_ = false;
  move_left_     = false;
  move_right_    = false;
}

void Camera::set_free_fly(bool value) { free_fly_ = value; }

glm::vec3 Camera::front_movement() const { return front_movement_; }

glm::vec3 Camera::right() const { return right_; }

glm::vec3 Camera::front() const { return front_; }

void Camera::set_max_movement_speed(float value)
{
  max_movement_speed_ = value;
}

float Camera::pitch() const { return pitch_; }

void Camera::set_pitch(float value)
{
  pitch_ = value;
  update_camera_vectors();
}

void Camera::set_near_plane(float value)
{
  near_plane_ = value;
  recalculate_projection_matrix();
}

float Camera::near_plane() const { return near_plane_; }

void Camera::set_far_plane(float value)
{
  far_plane_ = value;
  recalculate_projection_matrix();
}

float Camera::far_plane() const { return far_plane_; }

void Camera::set_aspect_ratio(float value)
{
  aspect_ratio_ = value;
  recalculate_projection_matrix();
}

float Camera::aspect_ratio() const { return aspect_ratio_; }

void Camera::recalculate_projection_matrix()
{
  projection_matrix_ = glm::perspective(glm::radians(zoom_),
                                        aspect_ratio_,
                                        near_plane_,
                                        far_plane_);
}

glm::mat4 Camera::projection_matrix() const { return projection_matrix_; }

void Camera::save(FILE *file) const
{
    write_value(file, position_);
    write_value(file, front_);
    write_value(file, front_movement_);
    write_value(file, world_up_);
    write_value(file, up_);
    write_value(file, right_);
    write_value(file, yaw_);
    write_value(file, pitch_);
    write_value(file, movement_speed_);
    write_value(file, mouse_sensitivity_);
    write_value(file, zoom_);
    write_value(file, free_fly_);
    write_value(file, near_plane_);
    write_value(file, far_plane_);
    write_value(file, aspect_ratio_);
    write_value(file, projection_matrix_);
}

void Camera::read(FILE *file)
{
  read_value(file, position_);
  read_value(file, front_);
  read_value(file, front_movement_);
  read_value(file, world_up_);
  read_value(file, up_);
  read_value(file, right_);
  read_value(file, yaw_);
  read_value(file, pitch_);
  read_value(file, movement_speed_);
  read_value(file, mouse_sensitivity_);
  read_value(file, zoom_);
  read_value(file, free_fly_);
  read_value(file, near_plane_);
  read_value(file, far_plane_);
  read_value(file, aspect_ratio_);
  read_value(file, projection_matrix_);
}

void Camera::set_enable_acceleration(bool value)
{
  is_enable_acceleration_ = value;
}

} // namespace dc
