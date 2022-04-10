#pragma once

#include "asset.hpp"
#include "asset_handle.hpp"
#include "audio_buffer.hpp"

#include <filesystem>
#include <memory>

namespace dc
{

class AudioAssetHandle : public AssetHandle
{
public:
  AudioAssetHandle(const std::filesystem::path &file_path, const Asset &asset);

  bool is_ready() const override;

  std::shared_ptr<AudioBuffer> get() const;

private:
  std::shared_ptr<AudioBuffer> audio_buffer_{};
};

std::shared_ptr<AssetHandle>
audio_asset_loader(const std::filesystem::path &file_path, const Asset &asset);

} // namespace dc
