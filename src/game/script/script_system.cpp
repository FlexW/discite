#include "script_system.hpp"
#include "assert.hpp"
#include "component_types.hpp"
#include "engine.hpp"
#include "game_layer.hpp"
#include "log.hpp"
#include "physic/physic_events.hpp"
#include "scene_events.hpp"
#include "script_component.hpp"
#include "script_engine.hpp"

#include <filesystem>
#include <memory>

namespace dc
{

void ScriptSystem::init()
{
  const auto game_layer = Engine::instance()->layer_stack()->layer<GameLayer>();
  if (game_layer)
  {
    const auto scene = game_layer->scene();
    if (scene)
    {
      scene_ = scene->get();
    }
  }

  script_engine_ = std::make_unique<ScriptEngine>();
  const auto game_assembly_path =
      Engine::instance()->base_directory() / "scripts" / "Game.dll";
  if (std::filesystem::exists(game_assembly_path))
  {
    script_engine_->load_game_assembly(game_assembly_path);
  }
  else
  {
    DC_LOG_WARN("No game assembly in {} found", game_assembly_path.string());
  }
}

void ScriptSystem::update(float delta_time)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  const auto &view = scene->all_entities_with<ScriptComponent>();
  for (const auto &entity : view)
  {
    auto &script_component = view.get<ScriptComponent>(entity);
    if (script_component.entity_script_)
    {
      script_component.entity_script_->on_update(delta_time);
    }
  }
}

void ScriptSystem::render(SceneRenderInfo & /*scene_render_info*/,
                          ViewRenderInfo & /*view_render_info*/)
{
}

bool ScriptSystem::on_event(const Event &event)
{
  const auto event_id = event.id();

  if (event_id == SceneLoadedEvent::id)
  {
    on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }
  else if (event_id == ComponentConstructEvent::id)
  {
    on_component_construct(
        dynamic_cast<const ComponentConstructEvent &>(event));
    return false;
  }
  else if (event_id == ComponentDestroyEvent::id)
  {
    on_component_destroy(dynamic_cast<const ComponentDestroyEvent &>(event));
    return false;
  }
  else if (event_id == EntityCollisionBeginEvent::id)
  {
    on_entity_collision_begin(
        dynamic_cast<const EntityCollisionBeginEvent &>(event));
    return false;
  }
  else if (event_id == EntityCollisionEndEvent::id)
  {
    on_entity_collision_end(
        dynamic_cast<const EntityCollisionEndEvent &>(event));
    return false;
  }
  else if (event_id == EntityTriggerBeginEvent::id)
  {
    on_entity_trigger_begin(
        dynamic_cast<const EntityTriggerBeginEvent &>(event));
    return false;
  }
  else if (event_id == EntityTriggerEndEvent::id)
  {
    on_entity_trigger_end(dynamic_cast<const EntityTriggerEndEvent &>(event));
    return false;
  }

  return false;
}

void ScriptSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
}

void ScriptSystem::on_component_construct(const ComponentConstructEvent &event)
{
  if (event.component_type_ != ComponentType::Script)
  {
    return;
  }

  auto       &script_component = event.entity_.component<ScriptComponent>();
  const auto &module_name      = script_component.module_name_;

  auto script_entity =
      script_engine_->construct_entity(event.entity_, module_name);
  if (script_entity)
  {
    script_entity->on_create();
    script_component.entity_script_ = std::move(script_entity);
  }
}

void ScriptSystem::on_component_destroy(const ComponentDestroyEvent & /*event*/)
{
}

void ScriptSystem::on_entity_collision_begin(
    const EntityCollisionBeginEvent &event)
{
  if (!event.collider_.has_component<ScriptComponent>())
  {
    return;
  }
  auto &component = event.collider_.component<ScriptComponent>();
  component.entity_script_->on_collison_begin(event.collidee_);
}

void ScriptSystem::on_entity_collision_end(const EntityCollisionEndEvent &event)
{
  if (!event.collider_.has_component<ScriptComponent>())
  {
    return;
  }
  auto &component = event.collider_.component<ScriptComponent>();
  component.entity_script_->on_collison_end(event.collidee_);
}

void ScriptSystem::on_entity_trigger_begin(const EntityTriggerBeginEvent &event)
{
  if (!event.trigger_.has_component<ScriptComponent>())
  {
    return;
  }
  auto &component = event.trigger_.component<ScriptComponent>();
  component.entity_script_->on_trigger_begin(event.other_);
}

void ScriptSystem::on_entity_trigger_end(const EntityTriggerEndEvent &event)
{
  if (!event.trigger_.has_component<ScriptComponent>())
  {
    return;
  }
  auto &component = event.trigger_.component<ScriptComponent>();
  component.entity_script_->on_trigger_end(event.other_);
}

} // namespace dc
