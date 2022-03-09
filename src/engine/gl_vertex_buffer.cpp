#include "gl_vertex_buffer.hpp"

#include <cassert>

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

std::size_t GlVertexBufferLayout::size() const { return size_; }

GlVertexBuffer::GlVertexBuffer() { glGenBuffers(1, &vertex_buffer_id_); }

GlVertexBuffer::~GlVertexBuffer()
{
  if (vertex_buffer_id_)
  {
    glDeleteBuffers(1, &vertex_buffer_id_);
  }
}

GLuint GlVertexBuffer::id() const { return vertex_buffer_id_; }

GlVertexBufferLayout GlVertexBuffer::layout() const { return layout_; }

GLsizei GlVertexBuffer::count() const { return vertex_count_; }

void GlVertexBuffer::bind()
{
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id_);
}

void GlVertexBuffer::unbind() { glBindBuffer(GL_ARRAY_BUFFER, 0); }

} // namespace dc
