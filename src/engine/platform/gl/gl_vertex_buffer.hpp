#pragma once

#include "assert.hpp"
#include "gl.hpp"
#include "graphic/vertex_buffer.hpp"
#include "graphic/vertex_buffer_layout.hpp"

#include <stdexcept>
#include <vector>

namespace dc
{

class GlVertexBuffer : public VertexBuffer
{
public:
  GlVertexBuffer(const void               *data,
                 std::size_t               size,
                 unsigned                  count,
                 const VertexBufferLayout &layout,
                 VertexBufferUsage usage = VertexBufferUsage::Unspecified);

  GlVertexBuffer(std::size_t              size,
                 const VertexBufferLayout layout,
                 VertexBufferUsage        usage = VertexBufferUsage::MapWrite);

  ~GlVertexBuffer();

  GLuint id() const;

  void write(const void *data, std::size_t size, std::size_t offset) override;

  VertexBufferLayout layout() const override;
  unsigned           count() const override;
  std::size_t        size() const override;

  void bind();
  void unbind();

private:
  GLuint id_{};

  VertexBufferLayout layout_;

  std::size_t vertex_count_{};
  std::size_t size_{};

  GlVertexBuffer(const GlVertexBuffer &) = delete;
  void operator=(const GlVertexBuffer &) = delete;
  GlVertexBuffer(GlVertexBuffer &&)      = delete;
  void operator=(GlVertexBuffer &&) = delete;

  void init(const void *data, VertexBufferUsage usage);
};

} // namespace dc
