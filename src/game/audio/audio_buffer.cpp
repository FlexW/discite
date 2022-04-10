#include "audio_buffer.hpp"
#include "assert.hpp"
#include "open_al_helper.hpp"

namespace
{

using namespace dc;

ALenum to_al(AudioFormatType format)
{
  switch (format)
  {
  case AudioFormatType::Mono8:
    return AL_FORMAT_MONO8;
  case AudioFormatType::Mono16:
    return AL_FORMAT_MONO16;
  case AudioFormatType::Stereo8:
    return AL_FORMAT_STEREO8;
  case AudioFormatType::Stereo16:
    return AL_FORMAT_STEREO16;
  }

  DC_FAIL("Can not handle audio format");
  return AL_FORMAT_STEREO16;
}

} // namespace

namespace dc
{

AudioBuffer::AudioBuffer(const AudioBufferInfo &audio_buffer_info)
{
  AL_CALL(alGenBuffers, 1, &buffer_);
  AL_CALL(alBufferData,
          buffer_,
          to_al(audio_buffer_info.format_),
          audio_buffer_info.data_.data(),
          audio_buffer_info.data_.size() *
              sizeof(decltype(*audio_buffer_info.data_.data())),
          audio_buffer_info.sample_rate_);
}

AudioBuffer::~AudioBuffer() { AL_CALL(alDeleteSources, 1, &buffer_); }

unsigned AudioBuffer::id() const { return buffer_; }

} // namespace dc
