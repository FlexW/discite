#include "audio_system.hpp"
#include "audio/audio_engine.hpp"
#include "audio/audio_listener_component.hpp"
#include "audio/audio_source.hpp"
#include "audio/audio_source_component.hpp"
#include "component_types.hpp"
#include "entity.hpp"
#include "scene_events.hpp"
#include <memory>

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

  auto source_view = scene->all_entities_with<AudioSourceComponent>();
  for (const auto &e : source_view)
  {
    const auto &component = source_view.get<AudioSourceComponent>(e);
    component.audio_source_->set_positon(Entity{e, scene}.position());
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
  else if (event_id == ComponentDestroyEvent::id)
  {
    on_component_destroy(dynamic_cast<const ComponentDestroyEvent &>(event));
  }

  return false;
}

void AudioSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;

  // create audio sources
  auto scene = scene_.lock();
  if (scene)
  {
    auto audio_source_view = scene->all_entities_with<AudioSourceComponent>();
    for (const auto &entity : audio_source_view)
    {
      add_audio_source(Entity{entity, scene});
    }
  }
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
    add_audio_source(event.entity_);
  }
}

void AudioSystem::on_component_destroy(const ComponentDestroyEvent &event)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  if (event.component_type_ == ComponentType::AudioSource)
  {
    remove_audio_source(event.entity_);
  }
}

void AudioSystem::add_audio_source(Entity entity)
{
  const auto audio_source = AudioEngine::get_instance()->create_source(entity);
  audio_sources_[entity.id()] = audio_source;
}

std::shared_ptr<AudioSource> AudioSystem::get_audio_source(Entity entity)
{
  const auto iter = audio_sources_.find(entity.id());
  if (iter != audio_sources_.end())
  {
    return iter->second;
  }
  return nullptr;
}

void AudioSystem::remove_audio_source(Entity entity)
{
  const auto iter = audio_sources_.find(entity.id());
  if (iter != audio_sources_.end())
  {
    audio_sources_.erase(iter);
  }
}

} // namespace dc
