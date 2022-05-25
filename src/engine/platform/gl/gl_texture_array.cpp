#include "gl_texture_array.hpp"
#include "assert.hpp"
#include "gl_helper.hpp"
#include "math.hpp"

namespace dc
{

GlTextureArray::GlTextureArray(const TextureArrayConfig &texture_array_config)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id_);

  DC_ASSERT(texture_array_config.count > 0, "Texture array data too small");

  const auto width         = texture_array_config.width;
  const auto height        = texture_array_config.height;
  const auto texture_count = texture_array_config.count;
  const auto sized_format  = texture_array_config.sized_format;

  const auto levels = texture_array_config.is_generate_mipmap
                          ? glm::log2(glm::max(width, height)) + 1
                          : 1;

  // reserve space for textures
  glTextureStorage3D(id_,
                     levels,
                     to_gl(sized_format),
                     width,
                     height,
                     texture_count);

  // upload texture data in case there is any
  for (std::size_t i = 0; i < static_cast<std::size_t>(texture_count); ++i)
  {
    if (i >= texture_array_config.data.size())
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
                        to_gl(texture_array_config.format),
                        texture_array_config.type,
                        texture_array_config.data[i]);
  }

  if (texture_array_config.border_color.has_value())
  {
    const auto border_color = texture_array_config.border_color.value();
    assert(border_color.size() == 4);

    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, border_color.data());
  }

  if (levels > 1 &&
      texture_array_config.min_filter == TextureFilterMode::Linear)
  {
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  }
  else
  {
    glTextureParameteri(id_,
                        GL_TEXTURE_MIN_FILTER,
                        to_gl(texture_array_config.min_filter));
  }
  glTextureParameteri(id_,
                      GL_TEXTURE_MAG_FILTER,
                      to_gl(texture_array_config.mag_filter));
  glTextureParameteri(id_,
                      GL_TEXTURE_WRAP_S,
                      to_gl(texture_array_config.wrap_s));
  glTextureParameteri(id_,
                      GL_TEXTURE_WRAP_T,
                      to_gl(texture_array_config.wrap_t));

  if (texture_array_config.is_generate_mipmap)
  {
    glGenerateTextureMipmap(id_);
  }
}

GlTextureArray::~GlTextureArray() { glDeleteTextures(1, &id_); }

GLuint GlTextureArray::id() const { return id_; }

void GlTextureArray::bind_unit(int unit) const { glBindTextureUnit(unit, id_); }

} // namespace dc
