#pragma once

#include "gl.hpp"

namespace dc
{

class GlRenderbuffer
{
public:
  GlRenderbuffer(GLenum internal_format, GLsizei width, GLsizei height);
  GlRenderbuffer(GLenum  internal_format,
                 GLsizei samples,
                 GLsizei width,
                 GLsizei height);
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
