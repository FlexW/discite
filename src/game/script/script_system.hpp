#pragma once

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

  void
  on_script_component_construct(const ScriptComponentConstructEvent &event);
  void on_script_component_destroy(const ScriptComponentDestroyEvent &event);
  void on_scene_loaded(const SceneLoadedEvent &event);
};

} // namespace dc
