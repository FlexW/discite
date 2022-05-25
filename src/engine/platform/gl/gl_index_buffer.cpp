#include "gl_index_buffer.hpp"

#include <cstddef>
#include <cstdint>

namespace dc
{

GlIndexBuffer::GlIndexBuffer(const std::vector<std::uint32_t> &indices)
    : count_{static_cast<GLsizei>(indices.size())}
{
  glCreateBuffers(1, &id_);
  glNamedBufferStorage(id_,
                       indices.size() * sizeof(std::uint32_t),
                       indices.data(),
                       0);
}

GlIndexBuffer::~GlIndexBuffer()
{
  if (id_)
  {
    glDeleteBuffers(1, &id_);
  }
}

GLuint GlIndexBuffer::id() const { return id_; }

void GlIndexBuffer::bind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_); }

void GlIndexBuffer::unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

unsigned GlIndexBuffer::count() const { return count_; }

} // namespace dc
