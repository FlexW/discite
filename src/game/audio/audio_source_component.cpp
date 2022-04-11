#include "audio_source_component.hpp"
#include "engine.hpp"
#include "serialization.hpp"

namespace dc
{

void AudioSourceComponent::save(FILE *file) const
{
  write_value(file, pitch_);
  write_value(file, gain_);
  write_value(file, velocity_);
  write_value(file, looping_);
  write_value(file, start_on_create_);

  std::string audio_asset_name;
  if (audio_asset_)
  {
    audio_asset_name = audio_asset_->asset().id();
  }
  write_string(file, audio_asset_name);
}

void AudioSourceComponent::read(FILE *file)
{
  read_value(file, pitch_);
  read_value(file, gain_);
  read_value(file, velocity_);
  read_value(file, looping_);
  read_value(file, start_on_create_);

  std::string audio_asset_name;
  read_string(file, audio_asset_name);
  if (!audio_asset_name.empty())
  {
    audio_asset_ = std::dynamic_pointer_cast<AudioAssetHandle>(
        Engine::instance()->asset_cache()->load_asset(Asset{audio_asset_name}));
  }
  else
  {
    audio_asset_ = nullptr;
  }
}

} // namespace dc
