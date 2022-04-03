#include "gl_vertex_buffer.hpp"



namespace dc
{

void GlVertexBufferLayout::push_float(unsigned count)
{
  GlVertexBufferLayoutElement element{};
  element.type  = GL_FLOAT;
  element.count = count;
  element.size  = element.count * sizeof(float);
  elements_.push_back(element);

  size_ += element.size;
}

void GlVertexBufferLayout::push_int(unsigned count)
{
  GlVertexBufferLayoutElement element{};
  element.type   = GL_INT;
  element.count  = count;
  element.size   = element.count * sizeof(int);
  elements_.push_back(element);

  size_ += element.size;
}

std::vector<GlVertexBufferLayoutElement> GlVertexBufferLayout::elements() const
{
  return elements_;
}

GLsizei GlVertexBufferLayout::size() const { return size_; }

GlVertexBuffer::~GlVertexBuffer()
{
  if (id_)
  {
    glDeleteBuffers(1, &id_);
  }
}

GLuint GlVertexBuffer::id() const { return id_; }

GlVertexBufferLayout GlVertexBuffer::layout() const { return layout_; }

GLsizei GlVertexBuffer::count() const { return vertex_count_; }

void GlVertexBuffer::bind() { glBindBuffer(GL_ARRAY_BUFFER, id_); }

void GlVertexBuffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

std::size_t GlVertexBuffer::size() const { return size_; }

void GlVertexBuffer::write(const void *data,
                           std::size_t size,
                           std::size_t offset)
{
  if ((offset + size) > size_)
  {
    DC_FAIL("Can not write to buffer of size {}, {} bytes of data at offset {}",
            size_,
            size,
            offset);
    return;
  }

  const auto mapped_data =
      glMapNamedBufferRange(id_,
                            offset,
                            size,
                            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
  std::memcpy(mapped_data, data, size);
  glUnmapNamedBuffer(id_);
}

} // namespace dc
