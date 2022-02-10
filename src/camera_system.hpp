#pragma once

#include "engine/event.hpp"
#include "scene.hpp"
#include "system.hpp"

class CameraSystem : public System
{
public:
  CameraSystem(std::weak_ptr<Scene> scene);
  ~CameraSystem() override;

  void init() override;
  void update(float delta_time) override;

  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info) override;

private:
  std::weak_ptr<Scene> scene_{};

  void shutdown();

  void on_mouse_movement(const Event &event);
  void on_window_resize(const Event &event);
};
