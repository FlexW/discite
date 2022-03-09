#pragma once

#include "math.hpp"

namespace dc
{

enum class CameraMovement
{
  Forward,
  Backward,
  Left,
  Right
};

class Camera
{
public:
  Camera();

  glm::mat4 view_matrix() const;

  void process_movement(const CameraMovement direction, const float delta_time);

  void process_rotation(float      xoffset,
                        float      yoffset,
                        const bool constrain_pitch = true);

  void process_scroll(float yoffset);

  float zoom() const { return zoom_; }

  void      set_position(const glm::vec3 &value);
  glm::vec3 position() const { return position_; }

  void set_free_fly(bool value);

  glm::vec3 right() const;
  glm::vec3 front_movement() const;
  glm::vec3 front() const;

  float pitch() const;
  void  set_pitch(float value);

  void set_movement_speed(float value);

  void  set_near_plane(float value);
  float near_plane() const;

  void  set_far_plane(float value);
  float far_plane() const;

  void  set_aspect_ratio(float value);
  float aspect_ratio() const;

  glm::mat4 projection_matrix() const;

  void save(FILE *file) const;
  void read(FILE *file);

private:
  glm::vec3 position_{0.0f};
  glm::vec3 front_{0.0f, 0.0f, -1.0f};
  glm::vec3 front_movement_{front_};
  glm::vec3 world_up_{0.0f, 1.0f, 0.0f};
  glm::vec3 up_{world_up_};
  glm::vec3 right_;

  float yaw_{-90.0f};
  float pitch_{0.0f};

  float movement_speed_{20.0f};
  float mouse_sensitivity_{0.2f};
  float zoom_{45.0f};

  bool free_fly_{true};

  float     near_plane_{0.1f};
  float     far_plane_{500.0f};
  float     aspect_ratio_{1280.0f / 1024.0f};
  glm::mat4 projection_matrix_{1.0f};

  void update_camera_vectors();
  void recalculate_projection_matrix();
};

} // namespace dc
