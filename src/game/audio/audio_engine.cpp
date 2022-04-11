#include "audio_engine.hpp"
#include "assert.hpp"
#include "log.hpp"
#include "math.hpp"
#include "open_al_helper.hpp"

#include <memory>

namespace dc
{

AudioEngine *AudioEngine::get_instance()
{
  static auto audio_engine = std::unique_ptr<AudioEngine>(new AudioEngine);
  return audio_engine.get();
}

AudioEngine::AudioEngine()
{
  open_al_device_ = alcOpenDevice(nullptr);
  if (!open_al_device_)
  {
    DC_LOG_ERROR("Can not open audio device");
    return;
  }

  if (!ALC_CALL(alcCreateContext,
                open_al_context_,
                open_al_device_,
                open_al_device_,
                nullptr) ||
      !open_al_context_)
  {
    DC_LOG_ERROR("Can not create audio context");
    return;
  }

  ALCboolean context_made_current = false;
  if (!ALC_CALL(alcMakeContextCurrent,
                context_made_current,
                open_al_device_,
                open_al_context_) ||
      context_made_current != ALC_TRUE)
  {
    DC_LOG_ERROR("Can not make audio context current");
  }
}

AudioEngine::~AudioEngine()
{
  ALCboolean context_made_current;
  if (!ALC_CALL(alcMakeContextCurrent,
                context_made_current,
                open_al_device_,
                nullptr))
  {
    DC_LOG_ERROR("Could not make audio context non current");
  }

  if (!ALC_CALL(alcDestroyContext, open_al_device_, open_al_context_))
  {
    DC_LOG_ERROR("Could not destroy audio context");
  }

  ALCboolean closed;
  if (!ALC_CALL(alcCloseDevice, closed, open_al_device_, open_al_device_))
  {
    DC_LOG_ERROR("Could not close audio device");
  }
}

std::shared_ptr<AudioSource> AudioEngine::create_source(Entity entity) const
{
  return std::make_shared<AudioSource>(entity);
}

std::shared_ptr<AudioBuffer>
AudioEngine::create_buffer(const AudioBufferInfo &audio_buffer_info) const
{
  return std::make_shared<AudioBuffer>(audio_buffer_info);
}

void AudioEngine::set_listener_position(const glm::vec3 &position)
{
  alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void AudioEngine::set_listener_orientation(const glm::vec3 &direction)
{
  // NOTE: Vectors don't need to be normalized
  float values[6] = {
      direction.x,
      direction.y,
      direction.z,
      0.0f,
      1.0f,
      0.0f,
  };
  alListenerfv(AL_ORIENTATION, values);
}
} // namespace dc
