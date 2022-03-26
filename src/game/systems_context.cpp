#include "systems_context.hpp"
#include "profiling.hpp"

namespace dc
{

void SystemsContext::init()
{
  DC_PROFILE_SCOPE("SystemsContext::init()");

  for (const auto &system : systems_)
  {
    system->init();
  }
}

void SystemsContext::shutdown()
{
  DC_PROFILE_SCOPE("SystemsContext::shutdown()");

  for (const auto &system : systems_)
  {
    system->shutdown();
  }
}

void SystemsContext::update(float delta_time)
{
  DC_PROFILE_SCOPE("SystemsContext::update()");

  for (const auto &system : systems_)
  {
    system->update(delta_time);
  }
}

void SystemsContext::render(SceneRenderInfo &scene_render_info,
                            ViewRenderInfo  &view_render_info)
{
  DC_PROFILE_SCOPE("SystemsContext::render()");

  for (const auto &system : systems_)
  {
    system->render(scene_render_info, view_render_info);
  }
}

bool SystemsContext::on_event(const Event &event)
{
  DC_PROFILE_SCOPE("SystemsContext::on_event()");

  for (const auto &system : systems_)
  {
    system->on_event(event);
  }

  return false;
}

} // namespace dc
