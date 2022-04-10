#pragma once

#include "audio_buffer.hpp"
#include "math.hpp"

#include <AL/al.h>

#include <memory>

namespace dc
{

enum class AudioSourceState
{
  Initial,
  Playing,
  Paused,
  Stopped,
};

class AudioSource
{
public:
  AudioSource();
  ~AudioSource();

  void set_positon(const glm::vec3 &value);
  void set_velocity(const glm::vec3 &value);
  void set_gain(float value);
  void set_pitch(float value);
  void set_looping(bool value);
  void set_buffer(std::shared_ptr<AudioBuffer> buffer);

  void             play();
  void             stop();
  AudioSourceState state() const;

private:
  ALuint                       source_{};
  std::shared_ptr<AudioBuffer> buffer_{};
};

} // namespace dc
