#include "gl_texture_array.hpp"
#include "math.hpp"



namespace dc
{

GlTextureArray::GlTextureArray() { glGenTextures(1, &id_); }

GlTextureArray::~GlTextureArray() { glDeleteTextures(1, &id_); }

GLuint GlTextureArray::id() const { return id_; }

void GlTextureArray::set_data(const TextureArrayData &texture_array_data)
{
  assert(texture_array_data.count > 0);

  glBindTexture(GL_TEXTURE_2D_ARRAY, id_);

  const auto width  = texture_array_data.width;
  const auto height        = texture_array_data.height;
  const auto texture_count   = texture_array_data.count;
  const auto sized_format    = texture_array_data.sized_format;

  if (texture_array_data.data.size() > 0)
  {
    assert(texture_count == static_cast<int>(texture_array_data.data.size()));

    const auto levels = texture_array_data.is_generate_mipmap
                            ? glm::log2(glm::max(width, height)) + 1
                            : 1;

    glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                   levels,
                   sized_format,
                   width,
                   height,
                   texture_count);

    for (int i = 0; i < texture_count; ++i)
    {
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
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
  }
  else
  {
    glTexImage3D(GL_TEXTURE_2D_ARRAY,
                 0,
                 sized_format,
                 width,
                 height,
                 texture_count,
                 0,
                 texture_array_data.format,
                 texture_array_data.type,
                 nullptr);
  }

  if (texture_array_data.border_color.has_value())
  {
    const auto border_color = texture_array_data.border_color.value();
    assert(border_color.size() == 4);

    glTexParameterfv(GL_TEXTURE_2D_ARRAY,
                     GL_TEXTURE_BORDER_COLOR,
                     border_color.data());
  }

  glTexParameteri(GL_TEXTURE_2D_ARRAY,
                  GL_TEXTURE_MIN_FILTER,
                  texture_array_data.min_filter);
  glTexParameteri(GL_TEXTURE_2D_ARRAY,
                  GL_TEXTURE_MAG_FILTER,
                  texture_array_data.mag_filter);
  glTexParameteri(GL_TEXTURE_2D_ARRAY,
                  GL_TEXTURE_WRAP_S,
                  texture_array_data.wrap_s);
  glTexParameteri(GL_TEXTURE_2D_ARRAY,
                  GL_TEXTURE_WRAP_T,
                  texture_array_data.wrap_t);

  if (texture_array_data.is_generate_mipmap)
  {
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
  }

  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}

void GlTextureArray::bind() { glBindTexture(GL_TEXTURE_2D_ARRAY, id_); }

void GlTextureArray::unbind() { glBindTexture(GL_TEXTURE_2D_ARRAY, 0); }

} // namespace dc
