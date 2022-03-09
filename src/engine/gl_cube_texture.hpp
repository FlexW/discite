#pragma once

#include "gl.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace dc
{

class GlCubeTexture
{
public:
  GlCubeTexture(const std::vector<std::uint8_t> &hdr_data);
  ~GlCubeTexture();

  void bind();
  void unbind();

private:
  GLuint id_{};
};

} // namespace dc
