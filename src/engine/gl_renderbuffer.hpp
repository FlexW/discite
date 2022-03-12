#pragma once

#include "gl.hpp"

namespace dc
{

struct GlRenderbufferConfig
{
  GLenum  sized_format_;
  GLsizei width_;
  GLsizei height_;
  GLuint  msaa_{0};
};

class GlRenderbuffer
{
public:
  GlRenderbuffer(const GlRenderbufferConfig &config);
  ~GlRenderbuffer();

  GLuint id() const;

private:
  GLuint id_{};

  GlRenderbuffer(const GlRenderbuffer &other) = delete;
  void operator=(const GlRenderbuffer &other) = delete;
  GlRenderbuffer(GlRenderbuffer &&other)      = delete;
  void operator=(GlRenderbuffer &&other) = delete;
};

} // namespace dc
