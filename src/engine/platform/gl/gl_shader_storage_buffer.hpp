#pragma once

#include "assert.hpp"
#include "gl.hpp"
#include "graphic/shader_storage_buffer.hpp"

#include <cstring>
#include <vector>

namespace dc
{

class GlShaderStorageBuffer : public ShaderStorageBuffer
{
public:
  GlShaderStorageBuffer(const void             *data,
                        std::size_t             size,
                        ShaderStorageAccessMode access_mode =
                            ShaderStorageAccessMode::Unspecifed);

  ~GlShaderStorageBuffer();

  void bind() const;
  void bind(GLuint index);
  void unbind() const;

  void unmap() override;

protected:
  void *map(std::size_t             offset,
            std::size_t             size,
            ShaderStorageAccessMode access_mode) override;

private:
  GLuint id_{};
};

} // namespace dc
