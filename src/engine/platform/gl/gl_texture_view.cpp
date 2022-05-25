#include "gl_texture_view.hpp"
#include "assert.hpp"
#include "gl_helper.hpp"
#include "gl_texture.hpp"

namespace dc
{

GlTextureView::GlTextureView(const TextureViewConfig &config)
    : texture_{config.texture_}
{
  DC_ASSERT(texture_ != nullptr, "Need a texture");
  const auto gl_texture = dynamic_cast<GlTexture *>(texture_.get());
  DC_ASSERT(gl_texture, "Not a gl texture");

  glGenTextures(1, &id_);

  glTextureView(id_,
                GL_TEXTURE_2D,
                gl_texture->id(),
                to_gl(texture_->sized_format()),
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
