#pragma once

#include "event.hpp"
#include "scene.hpp"
#include "system.hpp"
#include "window.hpp"

namespace dc
{

class CameraSystem : public System
{
public:
  CameraSystem(std::weak_ptr<Scene> scene);
  ~CameraSystem() override;

  void init() override;
  void update(float delta_time) override;

  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info) override;

  bool on_event(const Event &event) override;

private:
  std::weak_ptr<Scene> scene_{};

  void shutdown();

  bool on_mouse_movement(const MouseMovementEvent &event);
  bool on_window_resize(const WindowResizeEvent &event);
};

} // namespace dc
