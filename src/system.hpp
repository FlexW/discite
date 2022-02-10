#pragma once

#include "renderer.hpp"

class System
{
public:
  virtual ~System() = default;

  virtual void init()                   = 0;
  virtual void update(float delta_time) = 0;

  virtual void render(SceneRenderInfo &scene_render_info,
                      ViewRenderInfo  &view_render_info) = 0;
};
