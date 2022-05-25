#pragma once

#include "texture.hpp"

#include <optional>

namespace dc
{

struct TextureArrayConfig
{
  TextureArrayConfig(TextureSizedFormat sized_format,
                     TextureFormat      format,
                     std::size_t        width,
                     std::size_t        height,
                     unsigned           count)
      : sized_format{sized_format},

        format{format},
        width{width},
        height{height},
        count{count}
  {
  }

  std::vector<unsigned char *> data{};
  TextureSizedFormat           sized_format;
  TextureFormat                format;
  TextureDataType              type{TextureDataType::UnsignedByte};
  std::size_t                  width;
  std::size_t                  height;
  unsigned                     count;

  bool is_generate_mipmap{false};

  TextureFilterMode mag_filter{TextureFilterMode::Nearest};
  TextureFilterMode min_filter{TextureFilterMode::Nearest};
  TextureWrapMode   wrap_s{TextureWrapMode::ClampToBorder};
  TextureWrapMode   wrap_t{TextureWrapMode::ClampToBorder};

  std::optional<std::vector<float>> border_color;
};

class TextureArray
{
public:
  virtual ~TextureArray() = default;
};

} // namespace dc
