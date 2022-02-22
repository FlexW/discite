#include "transform_component.hpp"

namespace
{

glm::mat4 calculate_transform_matrix(const glm::vec3 &position,
                                     const glm::quat &rotation,
                                     const glm::vec3 &scale)
{
  glm::mat4 transform_matrix{1.0f};

  transform_matrix = glm::translate(transform_matrix, position);
  transform_matrix *= glm::toMat4(rotation);
  transform_matrix = glm::scale(transform_matrix, scale);

  return transform_matrix;
}

} // namespace

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
  rotation_ = glm::quat(value);
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
  const auto transform_matrix =
      calculate_transform_matrix(position_, rotation_, scale_);
  transform_matrix_ = parent_transform_matrix_ * transform_matrix;
}

void TransformComponent::set_parent_transform_matrix(const glm::mat4 &value)
{
  parent_transform_matrix_ = value;
  recalculate_transform_matrix();
}

glm::mat4 TransformComponent::parent_transform_matrix() const
{
  return parent_transform_matrix_;
}

glm::mat4 TransformComponent::local_transform_matrix() const
{
  return calculate_transform_matrix(position_, rotation_, scale_);
}
