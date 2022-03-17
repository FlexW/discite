#include "gl_texture_view.hpp"
#include "assert.hpp"

namespace dc
{

GlTextureView::GlTextureView(const GlTextureViewConfig &config)
    : texture_{config.texture_}
{
  DC_ASSERT(texture_ != nullptr, "Need a texture");

  glGenTextures(1, &id_);

  glTextureView(id_,
                GL_TEXTURE_2D,
                texture_->id(),
                texture_->sized_format(),
                config.min_level_,
                config.num_levels_,
                config.min_layer_,
                config.num_layers_);
}

GlTextureView::~GlTextureView() { glDeleteTextures(1, &id_); }

void GlTextureView::bind_unit(GLuint unit) const
{
  glBindTextureUnit(unit, id_);
}

GLuint GlTextureView::id() const { return id_; }

} // namespace dc
