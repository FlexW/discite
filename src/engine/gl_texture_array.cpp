#include "gl_texture_array.hpp"
#include "assert.hpp"
#include "math.hpp"

namespace dc
{

GlTextureArray::GlTextureArray(const GlTextureArrayConfig &config)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id_);

  DC_ASSERT(config.count > 0, "Texture array data too small");

  const auto width         = config.width;
  const auto height        = config.height;
  const auto texture_count = config.count;
  const auto sized_format  = config.sized_format;

  const auto levels =
      config.is_generate_mipmap ? glm::log2(glm::max(width, height)) + 1 : 1;

  // reserve space for textures
  glTextureStorage3D(id_, levels, sized_format, width, height, texture_count);

  // upload texture data in case there is any
  for (std::size_t i = 0; i < static_cast<std::size_t>(texture_count); ++i)
  {
    if (i >= config.data.size())
    {
      break;
    }
    glTextureSubImage3D(id_,
                        0,
                        0,
                        0,
                        i,
                        width,
                        height,
                        1,
                        config.format,
                        config.type,
                        config.data[i]);
  }

  if (config.border_color.has_value())
  {
    const auto border_color = config.border_color.value();
    assert(border_color.size() == 4);

    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, border_color.data());
  }

  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, config.min_filter);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, config.mag_filter);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, config.wrap_s);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, config.wrap_t);

  if (config.is_generate_mipmap)
  {
    glGenerateTextureMipmap(id_);
  }
}

GlTextureArray::~GlTextureArray() { glDeleteTextures(1, &id_); }

GLuint GlTextureArray::id() const { return id_; }

void GlTextureArray::bind_unit(int unit) const { glBindTextureUnit(unit, id_); }

} // namespace dc
