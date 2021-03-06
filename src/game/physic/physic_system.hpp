#pragma once

#include "physic_scene.hpp"
#include "scene.hpp"
#include "scene_events.hpp"
#include "system.hpp"

#include <memory>

namespace dc
{

class PhysicSystem : public System
{
public:
  void init() override;
  void update(float delta_time) override;
  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info) override;

  bool on_event(const Event &event) override;

private:
  std::weak_ptr<Scene>         scene_{};
  std::unique_ptr<PhysicScene> physic_scene_{};

  void create_physic_actors();

  void on_scene_loaded(const SceneLoadedEvent &event);
  void on_scene_unloaded(const SceneUnloadedEvent &event);

  void on_component_construct(const ComponentConstructEvent &event);
  void on_component_destroy(const ComponentDestroyEvent &event);
};

} // namespace dc
