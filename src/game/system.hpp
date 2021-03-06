#pragma once

#include "event.hpp"
#include "frame_data.hpp"

namespace dc
{

class System
{
public:
  virtual ~System() = default;

  virtual void init() = 0;
  virtual void shutdown() {}
  virtual void update(float delta_time) = 0;

  virtual void render(SceneRenderInfo &scene_render_info,
                      ViewRenderInfo  &view_render_info) = 0;

  virtual bool on_event(const Event & /*event*/) { return false; }
};

} // namespace dc
