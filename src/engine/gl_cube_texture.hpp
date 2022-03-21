#pragma once

#include "gl.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace dc
{

struct GlCubeTextureConfig
{
  GLuint width_{};
  GLuint height_{};

  GLenum wrap_s_{GL_CLAMP_TO_EDGE};
  GLenum wrap_t_{GL_CLAMP_TO_EDGE};
  GLenum wrap_r_{GL_CLAMP_TO_EDGE};

  GLenum min_filter_{GL_LINEAR_MIPMAP_LINEAR};
  GLenum mag_filter_{GL_LINEAR};

  GLenum format{GL_RGB};
  GLenum sized_format{GL_RGB16F};
  GLenum type_{GL_FLOAT};

  bool generate_mipmaps_{true};
};

class GlCubeTexture
{
public:
  explicit GlCubeTexture(const GlCubeTextureConfig &config);
  explicit GlCubeTexture(const std::vector<std::uint8_t> &hdr_data);
  ~GlCubeTexture();

  GLuint id() const;

  void bind_unit(int unit) const;

  GLuint mipmap_levels() const;

private:
  GLuint id_{};
  GLuint mipmap_levels_{};
};

} // namespace dc
