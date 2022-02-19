#include "entity.hpp"
#include "math.hpp"
#include "name_component.hpp"
#include "transform_component.hpp"

Entity::Entity(entt::entity entity_handle, std::weak_ptr<Scene> scene)
    : entity_handle_{entity_handle},
      scene_{scene}
{
}

std::string Entity::name() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return {};
  }

  const auto name_component =
      scene->registry().get<NameComponent>(entity_handle_);

  return name_component.name_;
}

void Entity::set_name(const std::string &name)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &name_component = scene->registry().get<NameComponent>(entity_handle_);
  name_component.name_ = name;
}

entt::entity Entity::entity_handle() const { return entity_handle_; }

glm::mat4 Entity::transform_matrix() const
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return glm::mat4{1.0f};
  }

  const auto &transform_component =
      scene->registry().get<TransformComponent>(entity_handle_);
  return transform_component.transform_matrix();
}

void Entity::set_transform(const glm::mat4 &transform)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  glm::vec3 positon{};
  glm::vec3 rotation{};
  glm::vec3 scale{};
  if (math::decompose_transform(transform, positon, rotation, scale))
  {
    auto &transform_component =
        scene->registry().get<TransformComponent>(entity_handle_);
    const auto original_rotation = transform_component.rotation();
    const auto delta_rotation    = rotation - original_rotation;
    transform_component.set_position(positon);
    transform_component.set_rotation(original_rotation + delta_rotation);
    transform_component.set_scale(scale);
  }
}
