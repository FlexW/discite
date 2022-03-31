#pragma once

#include "scene.hpp"
#include "uuid.hpp"


#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

namespace dc
{

class Entity
{
public:
  Entity() = default;
  Entity(entt::entity entity_handle, std::weak_ptr<Scene> scene);

  void          set_id(Uuid uuid);
  std::uint64_t id() const;

  void remove();

  template <typename TComponent, typename... TArgs>
  decltype(auto) add_component(TArgs &&...args)
  {
    if (const auto scene = scene_.lock())
    {
      return scene->registry_.emplace<TComponent>(entity_handle_,
                                                  std::forward<TArgs>(args)...);
    }

    throw std::runtime_error("Can not add component if no scene is set");
  }

  template <typename TComponent> void remove_component()
  {
    assert(has_component<TComponent>());

    if (const auto scene = scene_.lock())
    {
      scene->registry_.remove<TComponent>(entity_handle_);
    }
  }

  template <typename TComponent> bool has_component() const
  {
    if (const auto scene = scene_.lock())
    {
      return scene->registry_.all_of<TComponent>(entity_handle_);
    }
    return false;
  }

  template <typename TComponent> decltype(auto) component()
  {
    assert(has_component<TComponent>());

    if (const auto scene = scene_.lock())
    {
      return scene->registry_.get<TComponent>(entity_handle_);
    }

    throw std::runtime_error("Can not get component if no scene is set");
  }

  template <typename TComponent> decltype(auto) component() const
  {
    assert(has_component<TComponent>());

    if (const auto scene = scene_.lock())
    {
      return scene->registry_.get<TComponent>(entity_handle_);
    }

    throw std::runtime_error("Can not get component if no scene is set");
  }

  bool valid() const;

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

  void   set_parent(Entity parent);
  Entity parent() const;

  bool has_parent() const;
  bool has_childs() const;

  bool operator==(const Entity &other) const;
  bool operator!=(const Entity &other) const;

private:
  entt::entity         entity_handle_{entt::null};
  std::weak_ptr<Scene> scene_;
};

} // namespace dc
