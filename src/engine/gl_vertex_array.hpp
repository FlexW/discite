#pragma once

#include "gl.hpp"
#include "gl_index_buffer.hpp"
#include "gl_vertex_buffer.hpp"

#include <memory>

namespace dc
{

class GlVertexArray
{
public:
  GlVertexArray();
  ~GlVertexArray();

  void add_vertex_buffer(std::shared_ptr<GlVertexBuffer> vertex_buffer);
  void set_index_buffer(std::shared_ptr<GlIndexBuffer> index_buffer);

  void bind() const;
  void unbind() const;

  GlIndexBuffer *index_buffer() const;
  GLsizei        vertex_count() const;

private:
  GLuint   id_{};
  unsigned binding_point_{0};

  std::vector<std::shared_ptr<GlVertexBuffer>>  vertex_buffers_;
  std::shared_ptr<GlIndexBuffer>                index_buffer_;

  GlVertexArray(const GlVertexArray &) = delete;
  void operator=(const GlVertexArray &) = delete;

  GlVertexArray(GlVertexArray &&) = delete;
  void operator=(GlVertexArray &&) = delete;
};

} // namespace dc
