#include "gl_texture.hpp"
#include "image.hpp"
#include "math.hpp"

#include <gli/load_ktx.hpp>
#include <fmt/format.h>

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

namespace dc
{

GlTexture::GlTexture(const GlTextureConfig &config) : format_{config.format_}
{
  if (config.msaa_ == 0)
  {
    glCreateTextures(GL_TEXTURE_2D, 1, &id_);

    const auto mipmap_levels =
        config.generate_mipmaps_
            ? math::calc_mipmap_levels_2d(config.width_, config.height_)
            : 1;

    glTextureStorage2D(id_,
                       mipmap_levels,
                       config.sized_format_,
                       config.width_,
                       config.height_);
  }
  else
  {
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &id_);
    glTextureStorage2DMultisample(id_,
                                  config.msaa_,
                                  config.sized_format_,
                                  config.width_,
                                  config.height_,
                                  GL_TRUE);
  }
  if (config.data_)
  {
    glTextureSubImage2D(id_,
                        0,
                        0,
                        0,
                        config.width_,
                        config.height_,
                        config.format_,
                        config.type_,
                        config.data_);
  }

  // set texture parameters
  glTextureParameteri(id_, GL_TEXTURE_MAX_LEVEL, config.max_level_);
  if (config.msaa_ == 0)
  {
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, config.wrap_s_);
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, config.wrap_t_);
    glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, config.min_filter_);
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, config.mag_filter_);
  }

  // generate mipmaps if needed
  if (config.generate_mipmaps_ && config.msaa_ == 0)
  {
    glGenerateTextureMipmap(id_);
  }
}

GlTexture::~GlTexture() { glDeleteTextures(1, &id_); }

GLuint GlTexture::id() const { return id_; }

std::shared_ptr<GlTexture>
GlTexture::load_from_file(const std::filesystem::path &file_path)
{
  if (file_path.extension() == ".ktx")
  {
    const auto            texture_ktx = gli::load_ktx(file_path.string());
    const gli::gl         gl{gli::gl::PROFILE_KTX};
    const gli::gl::format format{
        gl.translate(texture_ktx.format(), texture_ktx.swizzles())};
    glm::tvec3<GLsizei> extent{texture_ktx.extent(0)};
    const auto          width  = extent.x;
    const auto          height = extent.y;

    GlTextureConfig config{};
    config.data_             = const_cast<void *>(texture_ktx.data(0, 0, 0));
    config.width_            = width;
    config.height_           = height;
    config.max_level_        = 0;
    config.min_filter_       = GL_LINEAR;
    config.mag_filter_       = GL_LINEAR;
    config.wrap_s_           = GL_CLAMP_TO_EDGE;
    config.wrap_t_           = GL_CLAMP_TO_EDGE;
    config.format_           = format.External;
    config.sized_format_     = format.Internal;
    config.type_             = format.Type;
    config.generate_mipmaps_ = false;
    return std::make_shared<GlTexture>(config);
  }
  else
  {
    const Image     image{file_path};
    GlTextureConfig config{};
    config.data_   = const_cast<unsigned char *>(image.data());
    config.width_  = image.width();
    config.height_ = image.height();

    if (image.channels_count() == 1)
    {
      config.format_       = GL_RED;
      config.sized_format_ = GL_R8;
    }
    else if (image.channels_count() == 3)
    {
      config.format_       = GL_RGB;
      config.sized_format_ = GL_RGB8;
    }
    else if (image.channels_count() == 4)
    {
      config.format_       = GL_RGBA;
      config.sized_format_ = GL_RGBA8;
    }
    else
    {
      throw std::runtime_error(fmt::format("Can not handle {} channels in {}",
                                           image.channels_count(),
                                           file_path.string()));
    }
    return std::make_shared<GlTexture>(config);
  }
}

void GlTexture::bind_unit(int unit) const { glBindTextureUnit(unit, id_); }

GLenum GlTexture::format() const { return format_; }

} // namespace dc
