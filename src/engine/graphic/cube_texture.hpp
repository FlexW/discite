#pragma once

#include "texture.hpp"

#include <cstddef>

namespace dc
{

struct CubeTextureConfig
{
  TextureDataType           data_type_{TextureDataType::Float};
  std::vector<std::uint8_t> data_{};

  std::size_t width_{};
  std::size_t height_{};

  TextureWrapMode wrap_s_{TextureWrapMode::ClampToEdge};
  TextureWrapMode wrap_t_{TextureWrapMode::ClampToEdge};
  TextureWrapMode wrap_r_{TextureWrapMode::ClampToEdge};

  TextureFilterMode min_filter_{TextureFilterMode::Linear};
  TextureFilterMode mag_filter_{TextureFilterMode::Linear};

  TextureFormat      format{TextureFormat::Rgb};
  TextureSizedFormat sized_format{TextureSizedFormat::Rgb16F};

  bool generate_mipmaps_{true};
};

class CubeTexture
{
public:
  virtual ~CubeTexture() = default;

  virtual unsigned mipmap_levels() const = 0;
};

} // namespace dc
