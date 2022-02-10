#pragma once

#include "system.hpp"

#include <entt/entt.hpp>

#include <memory>

class Entity;

class Scene : public std::enable_shared_from_this<Scene>
{
public:
  void init();
  void update(float delta_time);
  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info);

  template <typename TSystem, typename... TArgs>
  void create_system(TArgs &&...args)
  {
    systems_.emplace_back(
        std::make_unique<TSystem>(std::forward<TArgs>(args)...));
  }

  Entity create_entity(const std::string &name);

  entt::registry &registry();

private:
  entt::registry                       registry_;
  std::vector<std::unique_ptr<System>> systems_;

  void init_systems();
};
