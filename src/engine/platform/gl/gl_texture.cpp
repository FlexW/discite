#include "gl_texture.hpp"
#include "assert.hpp"
#include "gl_helper.hpp"
#include "math.hpp"

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>

namespace dc
{

GlTexture::GlTexture(const TextureConfig &config)
    : format_{config.format_},
      sized_format_(config.sized_format_),
      width_{config.width_},
      height_{config.height_}
{
  const auto gl_format       = to_gl(config.format_);
  const auto gl_sized_format = to_gl(config.sized_format_);

  if (config.msaa_ == 0)
  {
    glCreateTextures(GL_TEXTURE_2D, 1, &id_);

    mipmap_levels_ = config.generate_mipmaps_
                         ? math::calc_mipmap_levels_2d(width_, height_)
                         : 1;

    glTextureStorage2D(id_, mipmap_levels_, gl_sized_format, width_, height_);
  }
  else
  {
    glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &id_);
    glTextureStorage2DMultisample(id_,
                                  config.msaa_,
                                  gl_sized_format,
                                  width_,
                                  height_,
                                  GL_TRUE);
  }
  if (config.data_)
  {
    glTextureSubImage2D(id_,
                        0,
                        0,
                        0,
                        width_,
                        height_,
                        gl_format,
                        to_gl(config.data_type_),
                        config.data_);
  }

  // set texture parameters
  glTextureParameteri(id_, GL_TEXTURE_MAX_LEVEL, config.max_level_);
  if (config.msaa_ == 0)
  {
    glTextureParameteri(id_, GL_TEXTURE_WRAP_S, to_gl(config.wrap_s_));
    glTextureParameteri(id_, GL_TEXTURE_WRAP_T, to_gl(config.wrap_t_));
    if (mipmap_levels_ > 1 && config.min_filter_ == TextureFilterMode::Linear)
    {
      glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
      glTextureParameteri(id_,
                          GL_TEXTURE_MIN_FILTER,
                          to_gl(config.min_filter_));
    }
    glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, to_gl(config.mag_filter_));
  }

  // generate mipmaps if needed
  if (config.generate_mipmaps_ && config.msaa_ == 0)
  {
    glGenerateTextureMipmap(id_);
  }
}

GlTexture::~GlTexture() { glDeleteTextures(1, &id_); }

GLuint GlTexture::id() const { return id_; }

void GlTexture::bind_unit(int unit) const { glBindTextureUnit(unit, id_); }

TextureFormat GlTexture::format() const { return format_; }

TextureSizedFormat GlTexture::sized_format() const { return sized_format_; }

std::size_t GlTexture::width() const { return width_; }

std::size_t GlTexture::height() const { return height_; }

unsigned GlTexture::mipmap_levels() const { return mipmap_levels_; }

glm::ivec2 GlTexture::mipmap_size(unsigned level) const
{
  DC_ASSERT(level >= 0, "Negative mipmap levels are not supported");

  auto width = width_;
  auto height = height_;

  for (; level != 0; --level)
  {
    width /= 2;
    height /= 2;
  }

  return {width, height};
}

} // namespace dc
