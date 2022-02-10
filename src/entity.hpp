#pragma once

#include "entt/entity/entity.hpp"
#include "scene.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <utility>

class Entity
{
public:
  Entity() = default;
  Entity(entt::entity entity_handle, std::weak_ptr<Scene> scene);

  template <typename TComponent, typename... TArgs>
  decltype(auto) add_component(TArgs &&...args)
  {
    if (const auto scene = scene_.lock())
    {
      return scene->registry().emplace<TComponent>(
          entity_handle_,
          std::forward<TArgs>(args)...);
    }

    throw std::runtime_error("Can not add component if no scene is set");
  }

  template <typename TComponent> void remove_component()
  {
    assert(has_component<TComponent>());

    if (const auto scene = scene_.lock())
    {
      scene->registry().remove<TComponent>(entity_handle_);
    }
  }

  template <typename TComponent> bool has_component()
  {
    if (const auto scene = scene_.lock())
    {
      return scene->registry().all_of<TComponent>(entity_handle_);
    }
    return false;
  }

  template <typename TComponent> decltype(auto) component()
  {
    assert(has_component<TComponent>());

    if (const auto scene = scene_.lock())
    {
      return scene->registry().get<TComponent>(entity_handle_);
    }

    throw std::runtime_error("Can not get component if no scene is set");
  }

  bool valid() { return entity_handle_ != entt::null; }

private:
  entt::entity         entity_handle_{entt::null};
  std::weak_ptr<Scene> scene_;
};
