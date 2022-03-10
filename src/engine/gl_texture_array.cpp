#include "gl_texture_array.hpp"
#include "assert.hpp"
#include "math.hpp"

namespace dc
{

GlTextureArray::GlTextureArray(const GlTextureArrayConfig &texture_array_data)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &id_);

  DC_ASSERT(texture_array_data.count > 0, "Texture array data too small");

  const auto width         = texture_array_data.width;
  const auto height        = texture_array_data.height;
  const auto texture_count = texture_array_data.count;
  const auto sized_format  = texture_array_data.sized_format;

  const auto levels = texture_array_data.is_generate_mipmap
                          ? glm::log2(glm::max(width, height)) + 1
                          : 1;

  // reserve space for textures
  glTextureStorage3D(id_, levels, sized_format, width, height, texture_count);

  // upload texture data in case there is any
  for (std::size_t i = 0; i < static_cast<std::size_t>(texture_count); ++i)
  {
    if (i >= texture_array_data.data.size())
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
                        texture_array_data.format,
                        texture_array_data.type,
                        texture_array_data.data[i]);
  }

  if (texture_array_data.border_color.has_value())
  {
    const auto border_color = texture_array_data.border_color.value();
    assert(border_color.size() == 4);

    glTextureParameterfv(id_, GL_TEXTURE_BORDER_COLOR, border_color.data());
  }

  glTextureParameteri(id_,
                      GL_TEXTURE_MIN_FILTER,
                      texture_array_data.min_filter);
  glTextureParameteri(id_,
                      GL_TEXTURE_MAG_FILTER,
                      texture_array_data.mag_filter);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, texture_array_data.wrap_s);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, texture_array_data.wrap_t);

  if (texture_array_data.is_generate_mipmap)
  {
    glGenerateTextureMipmap(id_);
  }
}

GlTextureArray::~GlTextureArray() { glDeleteTextures(1, &id_); }

GLuint GlTextureArray::id() const { return id_; }

void GlTextureArray::bind_unit(int unit) const { glBindTextureUnit(unit, id_); }

} // namespace dc
