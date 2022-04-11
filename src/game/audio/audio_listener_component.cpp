#include "audio_listener_component.hpp"
#include "serialization.hpp"

namespace dc
{

void AudioListenerComponent::save(FILE *file) const
{
  write_value(file, active_);
}

void AudioListenerComponent::read(FILE *file) { read_value(file, active_); }

} // namespace dc
