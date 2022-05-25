#pragma once

#include "math.hpp"

#include <cstdint>

namespace dc
{

enum class TextureFormat
{
  R,
  Rgb,
  Rgba,
};

enum class TextureSizedFormat
{
  R8,
  Rgb8,
  Rgba8,
  Rgb16F,
};

enum class TextureWrapMode
{
  Repeat,
  ClampToEdge,
  ClampToBorder,
};

enum class TextureFilterMode
{
  Nearest,
  Linear,
};

enum TextureDataType
{
  UnsignedByte,
  Float,
};

struct TextureConfig
{
  TextureDataType data_type_{TextureDataType::UnsignedByte};
  void       *data_{};
  std::size_t width_;
  std::size_t height_;

  TextureFormat      format_{TextureFormat::Rgba};
  TextureSizedFormat sized_format_{TextureSizedFormat::Rgba8};

  unsigned msaa_{0};

  unsigned        max_level_{1000};
  TextureWrapMode wrap_s_{TextureWrapMode::Repeat};
  TextureWrapMode   wrap_t_{TextureWrapMode::Repeat};
  TextureFilterMode min_filter_{TextureFilterMode::Linear};
  TextureFilterMode mag_filter_{TextureFilterMode::Linear};

  bool generate_mipmaps_{true};
};

class Texture
{
public:
  virtual ~Texture() = default;

  virtual TextureFormat      format() const       = 0;
  virtual TextureSizedFormat sized_format() const = 0;

  virtual std::size_t width() const  = 0;
  virtual std::size_t height() const = 0;

  virtual unsigned   mipmap_levels() const             = 0;
  virtual glm::ivec2 mipmap_size(unsigned level) const = 0;
};

} // namespace dc
