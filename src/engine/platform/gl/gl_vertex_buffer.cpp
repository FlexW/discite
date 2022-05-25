#include "gl_vertex_buffer.hpp"
#include "graphic/vertex_buffer.hpp"

namespace
{

GLenum to_gl(dc::VertexBufferUsage usage)
{
  switch (usage)
  {
  case dc::VertexBufferUsage::Unspecified:
    return {};

  case dc::VertexBufferUsage::MapWrite:
    return GL_MAP_WRITE_BIT;

  case dc::VertexBufferUsage::MapRead:
    return GL_MAP_READ_BIT;

  case dc::VertexBufferUsage::MapReadWrite:
    return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;

  case dc::VertexBufferUsage::MapPersistentRead:
    return GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT;

  case dc::VertexBufferUsage::MapPersistentWrite:
    return GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT;

  case dc::VertexBufferUsage::MapPersistentReadWrite:
    return GL_MAP_PERSISTENT_BIT | GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
  }

  return {};
}

} // namespace

namespace dc
{

GlVertexBuffer::GlVertexBuffer(const void               *data,
                               std::size_t               size,
                               unsigned                  count,
                               const VertexBufferLayout &layout,
                               VertexBufferUsage         usage)
    : layout_{layout},
      vertex_count_{count},
      size_{size}
{
  init(data, usage);
}

GlVertexBuffer::GlVertexBuffer(std::size_t              size,
                               const VertexBufferLayout layout,
                               VertexBufferUsage        usage)
    : layout_{layout},
      size_{size}
{
  init(nullptr, usage);
}

GlVertexBuffer::~GlVertexBuffer()
{
  if (id_)
  {
    glDeleteBuffers(1, &id_);
  }
}

void GlVertexBuffer::init(const void *data, VertexBufferUsage usage)
{
  glCreateBuffers(1, &id_);
  glNamedBufferStorage(id_, size_, data, to_gl(usage));
}

GLuint GlVertexBuffer::id() const { return id_; }

VertexBufferLayout GlVertexBuffer::layout() const { return layout_; }

unsigned GlVertexBuffer::count() const { return vertex_count_; }

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
