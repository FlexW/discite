#include "audio_system.hpp"
#include "audio/audio_engine.hpp"
#include "audio/audio_listener_component.hpp"
#include "audio/audio_source.hpp"
#include "audio/audio_source_component.hpp"
#include "component_types.hpp"
#include "scene_events.hpp"

namespace dc
{

void AudioSystem::init() {}

void AudioSystem::update(float /*delta_time*/) {}

void AudioSystem::render(SceneRenderInfo & /*scene_render_info*/,
                         ViewRenderInfo & /*view_render_info*/)
{
  auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto listener_view = scene->all_entities_with<AudioListenerComponent>();
  for (const auto &e : listener_view)
  {
    const auto &component = listener_view.get<AudioListenerComponent>(e);
    if (component.active_)
    {
      Entity entity{e, scene};
      AudioEngine::get_instance()->set_listener_position(entity.position());
      // TODO: Set orientation
      break;
    }
  }
}

bool AudioSystem::on_event(const Event &event)
{
  const auto event_id = event.id();
  if (event_id == SceneLoadedEvent::id)
  {
    on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }
  else if (event_id == ComponentConstructEvent::id)
  {
    on_component_construct(
        dynamic_cast<const ComponentConstructEvent &>(event));
  }

  return false;
}

void AudioSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
}

void AudioSystem::on_component_construct(const ComponentConstructEvent &event)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  if (event.component_type_ == ComponentType::AudioSource)
  {
    auto &component = event.entity_.component<AudioSourceComponent>();

    const auto audio_source = AudioEngine::get_instance()->create_source();
    audio_source->set_gain(component.gain_);
    audio_source->set_looping(component.looping_);
    audio_source->set_pitch(component.pitch_);
    audio_source->set_velocity(component.velocity_);

    component.audio_source_ = audio_source;
  }
}

} // namespace dc
