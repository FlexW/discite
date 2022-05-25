#include "gl_shader_storage_buffer.hpp"

namespace dc
{

GlShaderStorageBuffer::GlShaderStorageBuffer(
    const void             *data,
    std::size_t             size,
    ShaderStorageAccessMode access_mode)
{
  GLenum flags{0};
  switch (access_mode)
  {
  case ShaderStorageAccessMode::MapRead:
    flags = GL_STATIC_READ;
    break;
  case ShaderStorageAccessMode::Unspecifed:
    break;
  default:
    DC_FAIL("Unsupported access mode");
  }

  glCreateBuffers(1, &id_);
  glNamedBufferData(id_, size, data, flags);
}

GlShaderStorageBuffer::~GlShaderStorageBuffer() { glDeleteBuffers(1, &id_); }

void GlShaderStorageBuffer::bind() const
{
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, id_);
}

void GlShaderStorageBuffer::bind(GLuint index)
{
  bind();
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, id_);
}

void GlShaderStorageBuffer::unbind() const
{
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void *GlShaderStorageBuffer::map(std::size_t             offset,
                                 std::size_t             size,
                                 ShaderStorageAccessMode access_mode)
{
  GLbitfield access_flags{GL_MAP_INVALIDATE_BUFFER_BIT};
  switch (access_mode)
  {
  case ShaderStorageAccessMode::MapRead:
    access_flags |= GL_MAP_READ_BIT;
    break;
  case ShaderStorageAccessMode::MapWrite:
    access_flags |= GL_MAP_WRITE_BIT;
    break;
  case ShaderStorageAccessMode::MapReadWrite:
  case ShaderStorageAccessMode::Unspecifed:
    access_flags |= (GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
    break;
  }

  return glMapNamedBufferRange(id_, offset, size, access_flags);
}

void GlShaderStorageBuffer::unmap() { glUnmapNamedBuffer(id_); }

} // namespace dc
