#include "gl_shader_storage_buffer.hpp"

namespace dc
{

GlShaderStorageBuffer::GlShaderStorageBuffer() { glGenBuffers(1, &id_); }

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

void GlShaderStorageBuffer::resize(GLuint size, GLenum usage)
{
  bind();
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, usage);
  unbind();
}

void GlShaderStorageBuffer::unmap()
{
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  unbind();
}

} // namespace dc
