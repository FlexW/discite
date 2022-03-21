#include "gl_cube_texture.hpp"
#include "gl_texture.hpp"
#include "image.hpp"

#include <cstdint>
#include <stb_image.h>

#include <stdexcept>

namespace
{

using namespace dc;

Image get_image(int width, int height, int channels_count, float *data)
{
  Image in{width,
           height,
           channels_count,
           ImageFormat::Float,
           reinterpret_cast<std::uint8_t *>(data)};
  auto  out = in.is_euqirectangular()
                  ? convert_equirectangular_map_to_vertical_cross(in)
                  : std::move(in);
  return out;
}

} // namespace

namespace dc
{

GlCubeTexture::GlCubeTexture(const GlCubeTextureConfig &config)
{
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id_);

  mipmap_levels_ =
      config.generate_mipmaps_
          ? math::calc_mipmap_levels_2d(config.width_, config.height_)
          : 1;

  glTextureStorage2D(id_,
                     mipmap_levels_,
                     config.sized_format,
                     config.width_,
                     config.height_);

  /* for (unsigned int i = 0; i < 6; ++i)
  {
    glTextureSubImage3D(id_,
                        0,
                        0,
                        0,
                        i,
                        config.width_,
                        config.height_,
                        1,
                        config.format,
                        config.type_,
                        nullptr);
  }*/
  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, config.wrap_s_);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, config.wrap_t_);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_R, config.wrap_r_);
  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, config.min_filter_);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, config.mag_filter_);
}

GlCubeTexture::GlCubeTexture(const std::vector<std::uint8_t> &hdr_data)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &id_);
  glTextureParameteri(id_, GL_TEXTURE_MAX_LEVEL, 0);
  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int        width{}, height{}, channels_count{};
  const auto data = stbi_loadf_from_memory(hdr_data.data(),
                                           hdr_data.size(),
                                           &width,
                                           &height,
                                           &channels_count,
                                           3);
  if (!data)
  {
    throw std::runtime_error("Could not load hdr data");
  }
  assert(data);
  const auto out = get_image(width, height, channels_count, data);
  stbi_image_free(reinterpret_cast<void *>(data));
  const auto cubemap = convert_vertical_cross_to_cube_map_faces(out);

  const auto mipmaps_count =
      math::calc_mipmap_levels_2d(cubemap.width(), cubemap.height());

  glTextureParameteri(id_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTextureParameteri(id_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTextureParameteri(id_, GL_TEXTURE_BASE_LEVEL, 0);
  glTextureParameteri(id_, GL_TEXTURE_MAX_LEVEL, mipmaps_count - 1);
  glTextureParameteri(id_, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTextureParameteri(id_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  glTextureStorage2D(id_,
                     mipmaps_count,
                     GL_RGB32F,
                     cubemap.width(),
                     cubemap.height());

  auto cubemap_data = cubemap.data();

  for (unsigned i = 0; i != 6; ++i)
  {
    glTextureSubImage3D(id_,
                        0,
                        0,
                        0,
                        i,
                        cubemap.width(),
                        cubemap.height(),
                        1,
                        GL_RGB,
                        GL_FLOAT,
                        cubemap_data);
    cubemap_data += cubemap.width() * cubemap.height() *
                    cubemap.channels_count() * cubemap.bytes_per_component();
  }

  glGenerateTextureMipmap(id_);
}

GlCubeTexture::~GlCubeTexture()
{
  if (id_)
  {
    glDeleteTextures(1, &id_);
  }
}

GLuint GlCubeTexture::id() const { return id_; }

void GlCubeTexture::bind_unit(int unit) const { glBindTextureUnit(unit, id_); }

GLuint GlCubeTexture::mipmap_levels() const { return mipmap_levels_; }

} // namespace dc
