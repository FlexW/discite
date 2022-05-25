#pragma once

#include "gl.hpp"
#include "graphic/renderbuffer.hpp"

namespace dc
{

class GlRenderbuffer : public Renderbuffer
{
public:
  GlRenderbuffer(const RenderbufferConfig &config);
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
