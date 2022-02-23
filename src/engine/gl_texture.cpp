#include "gl_texture.hpp"
#include "image.hpp"

#include <cstdint>
#include <gli/load_ktx.hpp>

#include <cassert>
#include <stdexcept>
#include <string>

int calc_mipmap_levels_2d(int width, int height)
{
  int levels{1};
  while ((width | height) >> levels)
  {
    levels += 1;
  }
  return levels;
}

GlTexture::GlTexture() { glGenTextures(1, &id_); }

GlTexture::~GlTexture() { glDeleteTextures(1, &id_); }

GLuint GlTexture::id() const { return id_; }

void GlTexture::load_from_file(const std::filesystem::path &file_path,
                               bool                         generate_mipmap)
{
  if (file_path.extension() == ".ktx")
  {
    const auto            texture_ktx = gli::load_ktx(file_path.string());
    const gli::gl         gl{gli::gl::PROFILE_KTX};
    const gli::gl::format format{
        gl.translate(texture_ktx.format(), texture_ktx.swizzles())};
    glm::tvec3<GLsizei> extent{texture_ktx.extent(0)};
    const auto          width  = extent.x;
    const auto          height        = extent.y;
    const auto          mipmap_levels = calc_mipmap_levels_2d(width, height);

    bind();

    glTextureParameteri(id_, GL_TEXTURE_MAX_LEVEL, 0);
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTextureStorage2D(id_, mipmap_levels, format.Internal, width, height);
    glTextureSubImage2D(id_,
                        0,
                        0,
                        0,
                        width,
                        height,
                        format.External,
                        format.Type,
                        texture_ktx.data(0, 0, 0));
    format_ = format.External;

    unbind();
  }
  else
  {
    const Image image{file_path};
    set_data(image.data(),
             image.width(),
             image.height(),
             image.channels_count(),
             generate_mipmap);
  }
}

void GlTexture::set_data(const std::uint8_t *data,
                         int                 width,
                         int                 height,
                         int                 channels_count,
                         bool                generate_mipmap)
{
  bind();

  // Figure out the image format
  GLint internal_format{};
  if (channels_count == 1)
  {
    format_         = GL_RED;
    internal_format = GL_RED;
  }
  else if (channels_count == 3)
  {
    format_         = GL_RGB;
    internal_format = GL_RGB;
  }
  else if (channels_count == 4)
  {
    format_         = GL_RGBA;
    internal_format = GL_RGBA;
  }
  else
  {
    throw std::runtime_error("Can not handle channel count " +
                             std::to_string(channels_count));
  }

  // Send the image data to the GPU
  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,
               width,
               height,
               0,
               format_,
               GL_UNSIGNED_BYTE,
               data);

  // Set texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (generate_mipmap)
  {
    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  else
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  // Generate mipmaps if needed
  if (generate_mipmap)
  {
    glGenerateMipmap(GL_TEXTURE_2D);
  }

  unbind();
}

void GlTexture::bind() { glBindTexture(GL_TEXTURE_2D, id_); }

void GlTexture::unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

void GlTexture::set_storage(GLsizei width,
                            GLsizei height,
                            GLint   internal_format,
                            GLenum  format)
{
  bind();

  glTexImage2D(GL_TEXTURE_2D,
               0,
               internal_format,
               width,
               height,
               0,
               format,
               GL_UNSIGNED_BYTE,
               nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  unbind();
}

GLenum GlTexture::format() const { return format_; }
