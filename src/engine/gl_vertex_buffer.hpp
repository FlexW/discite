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
  template <typename T>
  GlVertexBuffer(const std::vector<T>      &data,
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
    if ((offset + size) >= size_)
    {
      DC_FAIL(
          "Can not write to buffer of size {} at offset {} {} bytes of data",
          size_,
          offset,
          size);
      return;
    }

    const auto mapped_data =
        glMapNamedBufferRange(id_,
                              offset,
                              size,
                              GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    std::memcpy(mapped_data, data.data(), size);
    glUnmapNamedBuffer(id_);
  }

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
