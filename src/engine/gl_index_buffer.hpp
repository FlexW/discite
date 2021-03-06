#pragma once

#include "gl.hpp"

#include <cstdint>
#include <vector>

namespace dc
{

class GlIndexBuffer
{
public:
  GlIndexBuffer(const std::vector<std::uint32_t> &indices);
  ~GlIndexBuffer();

  void bind();
  void unbind();

  GLuint id() const;

  GLsizei count() const;

private:
  GLuint  id_{};
  GLsizei count_{};

  GlIndexBuffer(const GlIndexBuffer &) = delete;
  void operator=(const GlIndexBuffer &) = delete;
  GlIndexBuffer(GlIndexBuffer &&)       = delete;
  void operator=(GlIndexBuffer &&) = delete;
};

} // namespace dc
