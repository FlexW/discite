#pragma once

#include "gl.hpp"
#include "graphic/index_buffer.hpp"

#include <cstdint>
#include <vector>

namespace dc
{

class GlIndexBuffer : public IndexBuffer
{
public:
  GlIndexBuffer(const std::vector<std::uint32_t> &indices);
  ~GlIndexBuffer();

  void bind();
  void unbind();

  GLuint id() const;

  unsigned count() const override;

private:
  GLuint  id_{};
  GLsizei count_{};

  GlIndexBuffer(const GlIndexBuffer &) = delete;
  void operator=(const GlIndexBuffer &) = delete;
  GlIndexBuffer(GlIndexBuffer &&)       = delete;
  void operator=(GlIndexBuffer &&) = delete;
};

} // namespace dc
