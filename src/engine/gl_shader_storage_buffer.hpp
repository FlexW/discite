#pragma once

#include "gl.hpp"

#include <cstring>
#include <vector>

namespace dc
{

class GlShaderStorageBuffer
{
public:
  template <typename T>
  GlShaderStorageBuffer(const std::vector<T> &data, GLenum flags = 0)
  {
    glCreateBuffers(1, &id_);
    glNamedBufferData(id_, data.size() * sizeof(T), data.data(), flags);
  }
  ~GlShaderStorageBuffer();

  void bind() const;
  void bind(GLuint index);
  void unbind() const;

  template <typename T>
  T *map(std::size_t offset,
         std::size_t count,
         GLbitfield  access_flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT |
                                   GL_MAP_INVALIDATE_BUFFER_BIT)
  {
    return static_cast<T *>(
        glMapNamedBufferRange(id_, offset, count * sizeof(T), access_flags));
  }

  void unmap();

private:
  GLuint id_{};
};

} // namespace dc
