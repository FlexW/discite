#pragma once

#include "renderer.hpp"
#include "scene.hpp"
#include "system.hpp"

#include <memory>

class RenderSystem : public System
{
public:
  RenderSystem(std::weak_ptr<Scene> scene);

  void init() override;
  void update(float delta_time) override;
  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info) override;

private:
  std::weak_ptr<Scene> scene_{};
};
