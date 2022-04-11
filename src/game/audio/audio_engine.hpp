#pragma once

#include "audio_buffer.hpp"
#include "audio_source.hpp"

#include <AL/al.h>
#include <AL/alc.h>

#include <memory>

namespace dc
{

class AudioEngine
{
public:
  static AudioEngine *get_instance();
  ~AudioEngine();

  std::shared_ptr<AudioSource> create_source(Entity entity) const;

  std::shared_ptr<AudioBuffer>
  create_buffer(const AudioBufferInfo &audio_buffer_info) const;

  void set_listener_position(const glm::vec3 &position);
  void set_listener_orientation(const glm::vec3 &direction);

private:
  AudioEngine();

  ALCdevice  *open_al_device_{};
  ALCcontext *open_al_context_{};
};

} // namespace dc
