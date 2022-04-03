#pragma once

#include "assert.hpp"
#include "gl.hpp"

#include <stdexcept>
#include <vector>

namespace dc
{

struct GlVertexBufferLayoutElement
{
  GLsizei  size;
  unsigned count;
  GLenum   type;
};

class GlVertexBufferLayout
{
public:
  void push_float(unsigned count);
  void push_int(unsigned count);

  std::vector<GlVertexBufferLayoutElement> elements() const;

  GLsizei size() const;

private:
  GLsizei                                  size_ = 0;
  std::vector<GlVertexBufferLayoutElement> elements_;
};

class GlVertexBuffer
{
public:
  template <typename T>
  GlVertexBuffer(const std::vector<T> &     data,
                 const GlVertexBufferLayout layout,
                 GLenum                     flags = 0)
      : size_{data.size() * sizeof(T)}
  {
    glCreateBuffers(1, &id_);

    glNamedBufferStorage(id_, size_, data.data(), flags);

    vertex_count_ = data.size();
    layout_       = layout;
  }

  GlVertexBuffer(std::size_t                size,
                 const GlVertexBufferLayout layout,
                 GLenum                     flags = GL_MAP_WRITE_BIT)
      : size_{size}
  {

    glCreateBuffers(1, &id_);
    glNamedBufferStorage(id_, size, nullptr, flags);
    layout_ = layout;
  }

  template <typename T>
  void write(const std::vector<T> &data, std::size_t offset)
  {
    const auto size = data.size() * sizeof(T);
    write(data.data(), size, offset);
  }

  void write(const void *data, std::size_t size, std::size_t offset);

  ~GlVertexBuffer();

  GLuint id() const;

  GlVertexBufferLayout layout() const;

  GLsizei count() const;

  void bind();
  void unbind();

  std::size_t size() const;

private:
  GLuint id_{};

  GlVertexBufferLayout layout_;

  GLsizei     vertex_count_{};
  std::size_t size_{};

  GlVertexBuffer(const GlVertexBuffer &) = delete;
  void operator=(const GlVertexBuffer &) = delete;
  GlVertexBuffer(GlVertexBuffer &&)      = delete;
  void operator=(GlVertexBuffer &&) = delete;
};

} // namespace dc
