#pragma once

#include "gl.hpp"

#include <vector>

namespace dc
{

struct GlCubeTextureArrayConfig
{
  GLenum   sized_format{GL_DEPTH_COMPONENT32F};
  GLenum   format{GL_DEPTH_COMPONENT};
  GLenum   type{GL_FLOAT};
  GLsizei  width;
  GLsizei  height;
  unsigned count;

  bool is_generate_mipmap{false};

  GLenum mag_filter{GL_NEAREST};
  GLenum min_filter{GL_NEAREST};
  GLenum wrap_s{GL_CLAMP_TO_EDGE};
  GLenum wrap_t{GL_CLAMP_TO_EDGE};
  GLenum wrap_r{GL_CLAMP_TO_EDGE};
};

class GlCubeTextureArray
{
public:
  GlCubeTextureArray(const GlCubeTextureArrayConfig &config);
  ~GlCubeTextureArray();

  GLuint id() const;

  void bind_unit(int unit) const;

private:
  GLuint id_{};

  GlCubeTextureArray(const GlCubeTextureArray &) = delete;
  void operator=(const GlCubeTextureArray &) = delete;
  GlCubeTextureArray(GlCubeTextureArray &&)  = delete;
  void operator=(GlCubeTextureArray &&) = delete;
};

} // namespace dc
