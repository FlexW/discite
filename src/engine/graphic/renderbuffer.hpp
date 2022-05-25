#pragma once

#include "texture.hpp"

namespace dc
{

struct RenderbufferConfig
{
  TextureSizedFormat sized_format_;
  std::size_t        width_;
  std::size_t        height_;
  unsigned           msaa_{0};
};

class Renderbuffer
{
public:
  virtual ~Renderbuffer() = default;
};

} // namespace dc
