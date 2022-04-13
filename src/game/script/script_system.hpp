#pragma once

#include "physic/physic_events.hpp"
#include "scene.hpp"
#include "scene_events.hpp"
#include "script_engine.hpp"
#include "system.hpp"

#include <memory>

namespace dc
{
class ScriptSystem : public System
{
public:
  void init() override;
  void update(float delta_time) override;
  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info) override;

  bool on_event(const Event &event) override;

private:
  std::weak_ptr<Scene> scene_{};

  std::unique_ptr<ScriptEngine> script_engine_{};

  void on_component_construct(const ComponentConstructEvent &event);
  void on_component_destroy(const ComponentDestroyEvent &event);
  void on_scene_loaded(const SceneLoadedEvent &event);
  void on_entity_collision_begin(const EntityCollisionBeginEvent &event);
  void on_entity_collision_end(const EntityCollisionEndEvent &event);
  void on_entity_trigger_begin(const EntityTriggerBeginEvent &event);
  void on_entity_trigger_end(const EntityTriggerEndEvent &event);
};

} // namespace dc
