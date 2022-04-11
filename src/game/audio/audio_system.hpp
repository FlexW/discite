#pragma once

#include "audio_engine.hpp"
#include "audio_source.hpp"
#include "scene.hpp"
#include "scene_events.hpp"
#include "system.hpp"
#include "uuid.hpp"

#include <memory>
#include <unordered_map>

namespace dc
{

class AudioSystem : public System
{
public:
  void init() override;

  void update(float delta_time) override;

  void render(SceneRenderInfo &scene_render_info,
              ViewRenderInfo  &view_render_info) override;

  bool on_event(const Event &event) override;

private:
  std::weak_ptr<Scene> scene_{};

  std::unordered_map<Uuid, std::shared_ptr<AudioSource>> audio_sources_;

  void on_scene_loaded(const SceneLoadedEvent &event);
  void on_component_construct(const ComponentConstructEvent &event);
  void on_component_destroy(const ComponentDestroyEvent &event);

  void add_audio_source(Entity entity);
  std::shared_ptr<AudioSource> get_audio_source(Entity entity);
  void remove_audio_source(Entity entity);
};

} // namespace dc
