#include "script_system.hpp"
#include "engine.hpp"
#include "game_layer.hpp"
#include "log.hpp"
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
    scene_ = game_layer->scene()->get();
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
  else if (event_id == ScriptComponentConstructEvent::id)
  {
    on_script_component_construct(
        dynamic_cast<const ScriptComponentConstructEvent &>(event));
    return false;
  }
  else if (event_id == ScriptComponentDestroyEvent::id)
  {
    on_script_component_destroy(
        dynamic_cast<const ScriptComponentDestroyEvent &>(event));
    return false;
  }

  return false;
}

void ScriptSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
}

void ScriptSystem::on_script_component_construct(
    const ScriptComponentConstructEvent &event)
{
  auto       &script_component = event.entity_.component<ScriptComponent>();
  const auto &module_name      = script_component.module_name_;

  auto script_entity =
      script_engine_->construct_entity(event.entity_, module_name);
  if (script_entity)
  {
    script_component.entity_script_ = std::move(script_entity);
  }
}

void ScriptSystem::on_script_component_destroy(
    const ScriptComponentDestroyEvent & /*event*/)
{
}

} // namespace dc
