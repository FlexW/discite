#pragma once

#include "asset_description.hpp"

#include <filesystem>
#include <memory>
#include <vector>

namespace dc
{

class WavFile
{
public:
  std::string                name_;
  std::uint8_t               channels_;
  std::int32_t               sample_rate_;
  std::uint8_t               bits_per_sample_;
  std::size_t                size_;
  std::vector<unsigned char> data_;

  std::vector<std::uint8_t> original_wav_data_{};

  WavFile() = default;

  WavFile(const std::string                &name,
          const std::uint8_t                channels,
          const std::int32_t                sample_rate,
          const std::uint8_t                bits_per_sample,
          const std::size_t                 size,
          const std::vector<unsigned char> &data)
      : name_(name),
        channels_(channels),
        sample_rate_(sample_rate),
        bits_per_sample_(bits_per_sample),
        size_(size),
        data_(data)
  {
  }

  void save(const std::filesystem::path &file_path,
            const AssetDescription      &asset_description) const;
  AssetDescription read(const std::filesystem::path &file_path);
};

} // namespace dc
