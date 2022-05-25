#include "renderer.hpp"
#include "graphic/texture.hpp"
#include "image.hpp"
#include "platform/gl/gl.hpp"

#include <fmt/format.h>
#include <gli/load_ktx.hpp>

namespace
{
using namespace dc;

dc::TextureFormat gl_texture_format_to_texture_format(GLenum format) {}

TextureSizedFormat
gl_texture_sized_format_to_texture_sized_format(GLenum format)
{
}

TextureDataType gl_data_type_to_data_type(GLenum type) {}

} // namespace

namespace dc
{

std::shared_ptr<Texture>
Renderer::create_texture(const std::filesystem::path &file_path)
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

    TextureConfig config{};
    config.data_       = const_cast<void *>(texture_ktx.data(0, 0, 0));
    config.width_      = width;
    config.height_     = height;
    config.max_level_  = 0;
    config.min_filter_ = TextureFilterMode::Linear;
    config.mag_filter_ = TextureFilterMode::Linear;
    config.wrap_s_     = TextureWrapMode::ClampToEdge;
    config.wrap_t_     = TextureWrapMode::ClampToEdge;
    config.format_     = gl_texture_format_to_texture_format(format.External);
    config.sized_format_ =
        gl_texture_sized_format_to_texture_sized_format(format.Internal);
    config.data_type_        = gl_data_type_to_data_type(format.Type);
    config.generate_mipmaps_ = false;
    return create_texture(config);
  }
  else
  {
    const Image   image{file_path};
    TextureConfig config{};
    config.data_   = const_cast<unsigned char *>(image.data());
    config.width_  = image.width();
    config.height_ = image.height();

    if (image.channels_count() == 1)
    {
      config.format_       = TextureFormat::R;
      config.sized_format_ = TextureSizedFormat::R8;
    }
    else if (image.channels_count() == 3)
    {
      config.format_       = TextureFormat::Rgb;
      config.sized_format_ = TextureSizedFormat::Rgb8;
    }
    else if (image.channels_count() == 4)
    {
      config.format_       = TextureFormat::Rgba;
      config.sized_format_ = TextureSizedFormat::Rgba8;
    }
    else
    {
      throw std::runtime_error(fmt::format("Can not handle {} channels in {}",
                                           image.channels_count(),
                                           file_path.string()));
    }
    return create_texture(config);
  }
}

} // namespace dc
