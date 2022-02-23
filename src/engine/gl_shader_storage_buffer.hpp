#pragma once

#include "gl.hpp"

#include <cstring>
#include <vector>

class GlShaderStorageBuffer
{
public:
  GlShaderStorageBuffer();
  ~GlShaderStorageBuffer();

  void bind() const;
  void bind(GLuint index);
  void unbind() const;

  template <typename T>
  void set_data(const std::vector<T> &data, GLenum usage = GL_DYNAMIC_COPY)
  {
    bind();

    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 data.size() * sizeof(T),
                 nullptr,
                 usage);

    T *mapped = map<T>(0, data.size());
    std::memcpy(mapped, data.data(), data.size() * sizeof(T));

    unmap();
    unbind();
  }

  template <typename T>
  T *map(std::size_t offset,
         std::size_t count,
         GLbitfield  access_flags = GL_MAP_WRITE_BIT | GL_MAP_READ_BIT |
                                   GL_MAP_INVALIDATE_BUFFER_BIT)
  {
    bind();

    return static_cast<T *>(glMapBufferRange(GL_SHADER_STORAGE_BUFFER,
                                             offset,
                                             count * sizeof(T),
                                             access_flags));
  }

  void unmap();

  void resize(GLuint size, GLenum usage = GL_DYNAMIC_COPY);

private:
  GLuint id_{};
};
