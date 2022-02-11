#include "entity.hpp"

Entity::Entity(entt::entity entity_handle, std::weak_ptr<Scene> scene)
    : entity_handle_{entity_handle},
      scene_{scene}
{
}
