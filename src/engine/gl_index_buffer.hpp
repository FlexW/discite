#pragma once

#include "gl.hpp"

#include <vector>

namespace dc
{

class GlIndexBuffer
{
public:
  GlIndexBuffer();
  ~GlIndexBuffer();

  void set_data(const std::vector<unsigned> &indices);

  void bind();
  void unbind();

  GLuint id() const;

  GLsizei count() const;

private:
  GLuint  index_buffer_id_{};
  GLsizei count_{};

  GlIndexBuffer(const GlIndexBuffer &) = delete;
  void operator=(const GlIndexBuffer &) = delete;
  GlIndexBuffer(GlIndexBuffer &&)       = delete;
  void operator=(GlIndexBuffer &&) = delete;
};

} // namespace dc
