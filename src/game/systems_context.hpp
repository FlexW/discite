#pragma once

#include "system.hpp"

#include <memory>
#include <vector>

namespace dc
{

class SystemsContext
{
public:
  template <typename TSystem, typename... TArgs>
  void add_system(TArgs &&...args)
  {
    systems_.emplace_back(
        std::make_unique<TSystem>(std::forward<TArgs>(args)...));
  }

  void init();
  void shutdown();

  void update(float delta_time);

  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info);

  bool on_event(const Event &event);

private:
  std::vector<std::unique_ptr<System>> systems_;
};
} // namespace dc
