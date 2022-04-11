#include "audio_source.hpp"
#include "assert.hpp"
#include "audio/audio_source_component.hpp"
#include "entity.hpp"
#include "open_al_helper.hpp"

namespace
{
using namespace dc;

AudioSourceState to_state(ALint state)
{
  switch (state)
  {
  case AL_PLAYING:
    return AudioSourceState::Playing;
  case AL_PAUSED:
    return AudioSourceState::Paused;
  case AL_STOPPED:
    return AudioSourceState::Stopped;
  case AL_INITIAL:
    return AudioSourceState::Initial;
  }
  DC_FAIL("Can not handle state.");
}

} // namespace

namespace dc
{

AudioSource::AudioSource(Entity entity) : entity_{entity}
{
  AL_CALL(alGenSources, 1, &source_);

  DC_ASSERT(entity.has_component<AudioSourceComponent>(),
            "No audio source component");

  auto &component = entity.component<AudioSourceComponent>();
  DC_ASSERT(component.audio_source_ == nullptr, "Audio source is already set");
  component.audio_source_ = this;

  set_gain(component.gain_);
  set_pitch(component.pitch_);
  set_looping(component.looping_);
  set_velocity(component.velocity_);
  set_positon(entity_.position());

  if (component.audio_asset_ && component.audio_asset_->is_ready())
  {
    set_buffer(component.audio_asset_->get());
  }

  if (component.start_on_create_)
  {
    play();
  }
}

AudioSource::~AudioSource() { AL_CALL(alDeleteSources, 1, &source_); }

void AudioSource::set_positon(const glm::vec3 &value)
{
  AL_CALL(alSource3f, source_, AL_POSITION, value.x, value.y, value.z);
  entity_.set_position(value);
}

void AudioSource::set_velocity(const glm::vec3 &value)
{
  AL_CALL(alSource3f, source_, AL_VELOCITY, value.x, value.y, value.z);
  entity_.component<AudioSourceComponent>().velocity_ = value;
}

void AudioSource::set_gain(float value)
{
  AL_CALL(alSourcef, source_, AL_GAIN, value);
  entity_.component<AudioSourceComponent>().gain_ = value;
}

void AudioSource::set_pitch(float value)
{
  AL_CALL(alSourcef, source_, AL_PITCH, value);
  entity_.component<AudioSourceComponent>().pitch_ = value;
}

void AudioSource::set_looping(bool value)
{
  AL_CALL(alSourcei, source_, AL_LOOPING, value);
  entity_.component<AudioSourceComponent>().looping_ = value;
}

void AudioSource::set_buffer(std::shared_ptr<AudioBuffer> buffer)
{
  DC_ASSERT(buffer, "No buffer set");

  AL_CALL(alSourcei, source_, AL_BUFFER, buffer->id());
  buffer_ = buffer;
}

void AudioSource::play() { AL_CALL(alSourcePlay, source_); }

void AudioSource::stop() { AL_CALL(alSourceStop, source_); }

AudioSourceState AudioSource::state() const
{
  ALint al_state{};
  AL_CALL(alGetSourcei, source_, AL_SOURCE_STATE, &al_state);
  return to_state(al_state);
}

} // namespace dc
