#pragma once

#include <AL/al.h>

#include <vector>

namespace dc
{

enum class AudioFormatType
{
  Mono8,
  Mono16,
  Stereo8,
  Stereo16,
};

struct AudioBufferInfo
{
  std::vector<unsigned char> data_{};
  AudioFormatType            format_;
  int                        sample_rate_;
};

class AudioBuffer
{
public:
  AudioBuffer(const AudioBufferInfo &audio_buffer_info);
  ~AudioBuffer();

  unsigned id() const;

private:
  ALuint buffer_{};
};

} // namespace dc
