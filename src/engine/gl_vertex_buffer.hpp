#pragma once

#include "gl.hpp"

#include <vector>

namespace dc
{

struct GlVertexBufferLayoutElement
{
  GLsizei  size;
  unsigned count;
  GLenum  type;
};

class GlVertexBufferLayout
{
public:
  void push_float(unsigned count);
  void push_int(unsigned count);

  std::vector<GlVertexBufferLayoutElement> elements() const;

  std::size_t size() const;

private:
  std::size_t                              size_ = 0;
  std::vector<GlVertexBufferLayoutElement> elements_;
};

class GlVertexBuffer
{
public:
  GlVertexBuffer();
  ~GlVertexBuffer();

  GLuint id() const;

  template <typename T>
  void set_data(const std::vector<T>      &data,
                const GlVertexBufferLayout layout,
                GLenum                     usage = GL_STATIC_DRAW)
  {
    bind();
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(T), data.data(), usage);
    unbind();

    vertex_count_ = data.size();
    layout_       = layout;
  }

  GlVertexBufferLayout layout() const;

  GLsizei count() const;

  void bind();
  void unbind();

private:
  GLuint vertex_buffer_id_{};

  GlVertexBufferLayout layout_;

  GLsizei vertex_count_{};

  GlVertexBuffer(const GlVertexBuffer &) = delete;
  void operator=(const GlVertexBuffer &) = delete;
  GlVertexBuffer(GlVertexBuffer &&)      = delete;
  void operator=(GlVertexBuffer &&) = delete;
};

} // namespace dc
