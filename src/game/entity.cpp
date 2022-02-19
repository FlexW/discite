#include "entity.hpp"
#include "name_component.hpp"

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
