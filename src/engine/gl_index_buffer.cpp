#include "gl_index_buffer.hpp"

GlIndexBuffer::GlIndexBuffer() { glGenBuffers(1, &index_buffer_id_); }

GlIndexBuffer::~GlIndexBuffer()
{
  if (index_buffer_id_)
  {
    glDeleteBuffers(1, &index_buffer_id_);
  }
}

GLuint GlIndexBuffer::id() const { return index_buffer_id_; }

void GlIndexBuffer::bind()
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id_);
}

void GlIndexBuffer::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

void GlIndexBuffer::set_data(const std::vector<unsigned> &indices)
{
  bind();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_id_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               indices.size() * sizeof(unsigned),
               indices.data(),
               GL_STATIC_DRAW);
  unbind();

  count_ = indices.size();
}

GLsizei GlIndexBuffer::count() const { return count_; }
