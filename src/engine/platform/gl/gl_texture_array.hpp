#pragma once

#include "gl.hpp"
#include "graphic/texture_array.hpp"

namespace dc
{

class GlTextureArray : public TextureArray
{
public:
  GlTextureArray(const TextureArrayConfig &texture_array_config);
  ~GlTextureArray();

  GLuint id() const;

  void bind_unit(int unit) const;

private:
  GLuint id_{};

  GlTextureArray(const GlTextureArray &) = delete;
  void operator=(const GlTextureArray &) = delete;
  GlTextureArray(GlTextureArray &&)      = delete;
  void operator=(GlTextureArray &&) = delete;
};

} // namespace dc
