#include "audio_asset.hpp"
#include "asset_handle.hpp"
#include "audio/audio_engine.hpp"
#include "audio_buffer.hpp"
#include "log.hpp"
#include "wav_file.hpp"

#include <memory>
#include <stdexcept>

namespace dc
{

AudioAssetHandle::AudioAssetHandle(const std::filesystem::path &file_path,
                                   const Asset                 &asset)
    : AssetHandle{asset}
{
  try
  {
    WavFile wav_file{};
    wav_file.read(file_path);

    AudioBufferInfo audio_buffer_info{};

    const auto channels        = wav_file.channels_;
    const auto bits_per_sample = wav_file.bits_per_sample_;

    DC_LOG_DEBUG("Wav file {} has {} channels and {} bits per sample",
                 file_path,
                 channels,
                 bits_per_sample);

    if (channels == 1 && bits_per_sample == 8)
    {
      DC_LOG_DEBUG("Load {} as Mono8", file_path.string());
      audio_buffer_info.format_ = AudioFormatType::Mono8;
    }
    else if (channels == 1 && bits_per_sample == 16)
    {
      DC_LOG_DEBUG("Load {} as Mono16", file_path.string());
      audio_buffer_info.format_ = AudioFormatType::Mono16;
    }
    else if (channels == 2 && bits_per_sample == 8)
    {
      DC_LOG_DEBUG("Load {} as Stereo8", file_path.string());
      audio_buffer_info.format_ = AudioFormatType::Stereo8;
    }
    else if (channels == 2 && bits_per_sample == 16)
    {
      DC_LOG_DEBUG("Load {} as Stereo16", file_path.string());
      audio_buffer_info.format_ = AudioFormatType::Stereo16;
    }
    else
    {
      DC_LOG_WARN(
          "Unrecognized audio format with channels {} and sample rate {} in {}",
          channels,
          bits_per_sample,
          file_path.string());
      return;
    }

    audio_buffer_info.data_        = wav_file.data_;
    audio_buffer_info.sample_rate_ = wav_file.sample_rate_;
    audio_buffer_ =
        AudioEngine::get_instance()->create_buffer(audio_buffer_info);
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not load audio asset {}: {}",
                file_path.string(),
                error.what());
  }
}

bool AudioAssetHandle::is_ready() const { return audio_buffer_ != nullptr; }

std::shared_ptr<AudioBuffer> AudioAssetHandle::get() const
{
  return audio_buffer_;
}

std::shared_ptr<AssetHandle>
audio_asset_loader(const std::filesystem::path &file_path, const Asset &asset)
{
  return std::make_shared<AudioAssetHandle>(file_path, asset);
}

} // namespace dc
