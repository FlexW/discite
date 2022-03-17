#pragma once

#include "gl.hpp"
#include "gl_texture.hpp"

namespace dc
{

struct GlTextureViewConfig
{
  std::shared_ptr<GlTexture> texture_{};
  GLuint                     min_level_{};
  GLuint                     max_level_{};
  GLuint                     num_levels_{};
  GLuint                     min_layer_{};
  GLuint                     num_layers_{};
};

class GlTextureView
{
public:
  GlTextureView(const GlTextureViewConfig &config);
  ~GlTextureView();

  void bind_unit(GLuint unit) const;

  GLuint id() const;

private:
  GLuint                     id_{};
  std::shared_ptr<GlTexture> texture_{};
};

} // namespace dc
