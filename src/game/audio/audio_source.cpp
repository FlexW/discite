#include "audio_source.hpp"
#include "assert.hpp"
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

AudioSource::AudioSource() { AL_CALL(alGenSources, 1, &source_); }

AudioSource::~AudioSource() { AL_CALL(alDeleteSources, 1, &source_); }

void AudioSource::set_positon(const glm::vec3 &value)
{
  AL_CALL(alSource3f, source_, AL_POSITION, value.x, value.y, value.z);
}

void AudioSource::set_velocity(const glm::vec3 &value)
{
  AL_CALL(alSource3f, source_, AL_VELOCITY, value.x, value.y, value.z);
}

void AudioSource::set_gain(float value)
{
  AL_CALL(alSourcef, source_, AL_GAIN, value);
}

void AudioSource::set_pitch(float value)
{
  AL_CALL(alSourcef, source_, AL_PITCH, value);
}

void AudioSource::set_looping(bool value)
{
  AL_CALL(alSourcei, source_, AL_LOOPING, value);
}

void AudioSource::set_buffer(std::shared_ptr<AudioBuffer> buffer)
{
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
