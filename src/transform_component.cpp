#include "transform_component.hpp"

void TransformComponent::set_position(const glm::vec3 &value)
{
  position_ = value;
  recalculate_transform_matrix();
}

glm::vec3 TransformComponent::position() const { return position_; }

void TransformComponent::set_rotation(const glm::quat &value)
{
  rotation_ = value;
  recalculate_transform_matrix();
}

glm::quat TransformComponent::rotation_quat() const { return rotation_; }

void TransformComponent::set_rotation(const glm::vec3 &value)
{
  const auto transform_x = glm::eulerAngleX(value.x);
  const auto transform_y = glm::eulerAngleY(value.y);
  const auto transform_z = glm::eulerAngleZ(value.z);

  const auto transform = transform_x * transform_y * transform_z;

  rotation_ = glm::toQuat(transform);
  recalculate_transform_matrix();
}

glm::vec3 TransformComponent::rotation() const
{
  return glm::eulerAngles(rotation_);
}

void TransformComponent::set_scale(const glm::vec3 &value)
{
  scale_ = value;
  recalculate_transform_matrix();
}

glm::vec3 TransformComponent::scale() const { return scale_; }

glm::mat4 TransformComponent::transform_matrix() const
{
  return transform_matrix_;
}

void TransformComponent::recalculate_transform_matrix()
{
  glm::mat4 transform_matrix{1.0f};

  transform_matrix = glm::translate(transform_matrix, position_);
  transform_matrix *= glm::toMat4(rotation_);
  transform_matrix = glm::scale(transform_matrix, scale_);

  transform_matrix_ = transform_matrix;
}
