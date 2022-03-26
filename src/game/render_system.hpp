#pragma once

#include "event.hpp"
#include "frame_data.hpp"
#include "scene.hpp"
#include "scene_events.hpp"
#include "system.hpp"

#include <memory>

namespace dc
{

class RenderSystem : public System
{
public:
  void init() override;
  void update(float delta_time) override;
  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info) override;

  bool on_event(const Event &event) override;

private:
  std::weak_ptr<Scene> scene_{};

  void on_scene_loaded(const SceneLoadedEvent &event);
};

} // namespace dc
