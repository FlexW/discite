#pragma once

#include "gl.hpp"
#include "graphic/texture_view.hpp"

namespace dc
{

class GlTextureView : public TextureView
{
public:
  GlTextureView(const TextureViewConfig &config);
  ~GlTextureView();

  void bind_unit(GLuint unit) const;

  GLuint id() const;

private:
  GLuint                   id_{};
  std::shared_ptr<Texture> texture_{};
};

} // namespace dc
