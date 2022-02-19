#pragma once

#include "entt/entity/entity.hpp"
#include "entt/entity/fwd.hpp"
#include "scene.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
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

  entt::entity entity_handle() const;

  void        set_name(const std::string &name);
  std::string name() const;

  glm::mat4 transform_matrix() const;

  void      set_local_transform_matrix(const glm::mat4 &transform);
  glm::mat4 local_transform_matrix() const;

  void      set_position(const glm::vec3 &positon);
  glm::vec3 position() const;

  void      set_rotation(const glm::vec3 &rotation);
  glm::vec3 rotation() const;

  void      set_scale(const glm::vec3 &scale);
  glm::vec3 scale() const;

  void add_child(Entity child);
  void set_parent(Entity parent);

private:
  entt::entity         entity_handle_{entt::null};
  std::weak_ptr<Scene> scene_;
};
