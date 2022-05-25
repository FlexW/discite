#pragma once

#include "texture.hpp"

#include <memory>

namespace dc
{

struct TextureViewConfig
{
  std::shared_ptr<Texture> texture_{};
  unsigned                 min_level_{};
  unsigned                 max_level_{};
  unsigned                 num_levels_{};
  unsigned                 min_layer_{};
  unsigned                 num_layers_{};
};

class TextureView
{
public:
  virtual ~TextureView() = default;
};

} // namespace dc
