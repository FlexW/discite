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

  void bind_vertex_buffers(
      const std::vector<const GlVertexBuffer *> &vertex_buffers);
  void unbind_vertex_buffers();

  void bind_index_buffer(const GlIndexBuffer &index_buffer);
  void unbind_index_buffer();

  void bind() const;
  void unbind() const;

  // GlIndexBuffer *index_buffer() const;
  // GLsizei        vertex_count() const;

private:
  GLuint   id_{};

  GlVertexArray(const GlVertexArray &) = delete;
  void operator=(const GlVertexArray &) = delete;

  GlVertexArray(GlVertexArray &&) = delete;
  void operator=(GlVertexArray &&) = delete;
};

} // namespace dc
