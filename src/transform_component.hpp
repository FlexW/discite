#pragma once

#include "engine/math.hpp"

struct TransformComponent
{

  void      set_position(const glm::vec3 &value);
  glm::vec3 position() const;

  void      set_rotation(const glm::quat &value);
  glm::quat rotation_quat() const;

  void      set_rotation(const glm::vec3 &value);
  glm::vec3 rotation() const;

  void      set_scale(const glm::vec3 &value);
  glm::vec3 scale() const;

  glm::mat4 transform_matrix() const;

private:
  glm::vec3 position_{0.0f};
  glm::quat rotation_{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 scale_{1.0f};

  glm::mat4 transform_matrix_{1.0f};

  void recalculate_transform_matrix();
};
