#pragma once

#include "math.hpp"

#include <bits/types/FILE.h>

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

  void set_parent_transform_matrix(const glm::mat4 &value);

  glm::mat4 transform_matrix() const;
  glm::mat4 local_transform_matrix() const;

  void save(FILE *file) const;
  void read(FILE *file);

private:
  glm::vec3 position_{0.0f};
  glm::quat rotation_{1.0f, 0.0f, 0.0f, 0.0f};
  glm::vec3 scale_{1.0f};

  glm::mat4 parent_transform_matrix_{1.0f};
  glm::mat4 transform_matrix_{1.0f};

  void recalculate_transform_matrix();
};
