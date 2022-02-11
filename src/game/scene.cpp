#include "scene.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "render_system.hpp"
#include "tag_component.hpp"
#include "transform_component.hpp"

void Scene::init_systems()
{
  systems_.emplace_back(std::make_unique<RenderSystem>(shared_from_this()));
}

void Scene::init()
{
  init_systems();

  for (const auto &system : systems_)
  {
    system->init();
  }
}

void Scene::update(float delta_time)
{
  for (const auto &system : systems_)
  {
    system->update(delta_time);
  }
}

void Scene::render(SceneRenderInfo &scene_render_info,
                   ViewRenderInfo  &view_render_info)
{
  for (const auto &system : systems_)
  {
    system->render(scene_render_info, view_render_info);
  }
}

bool Scene::on_event(const Event &event)
{
  for (const auto &system : systems_)
  {
    system->on_event(event);
  }

  return false;
}

entt::registry &Scene::registry() { return registry_; }

Entity Scene::create_entity(const std::string &name)
{
  Entity     entity{registry_.create(), shared_from_this()};
  const auto entity_name = name.empty() ? "Entity" : name;
  entity.add_component<TagComponent>(name);
  entity.add_component<TransformComponent>();
  return entity;
}
