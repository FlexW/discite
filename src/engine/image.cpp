#include "image.hpp"
#include "math.hpp"

#include <stb_image.h>

#include <cstring>
#include <stdexcept>

namespace
{

glm::vec3 face_coords_to_xyz(int i, int j, int faceID, int faceSize)
{
  const float A = 2.0f * float(i) / faceSize;
  const float B = 2.0f * float(j) / faceSize;

  if (faceID == 0)
  {
    return glm::vec3(-1.0f, A - 1.0f, B - 1.0f);
  }
  if (faceID == 1)
  {
    return glm::vec3(A - 1.0f, -1.0f, 1.0f - B);
  }
  if (faceID == 2)
  {
    return glm::vec3(1.0f, A - 1.0f, 1.0f - B);
  }
  if (faceID == 3)
  {
    return glm::vec3(1.0f - A, 1.0f, 1.0f - B);
  }
  if (faceID == 4)
  {
    return glm::vec3(B - 1.0f, A - 1.0f, 1.0f);
  }
  if (faceID == 5)
  {
    return glm::vec3(1.0f - B, A - 1.0f, -1.0f);
  }

  return {};
}

} // namespace

namespace dc
{

Image convert_equirectangular_map_to_vertical_cross(const Image &b)
{
  assert(b.is_euqirectangular());

  if (b.type() != ImageType::TwoD)
  {
    return {};
  }

  const int faceSize = b.width() / 4;

  const int w = faceSize * 3;
  const int h = faceSize * 4;

  Image result(w, h, b.channels_count(), b.format());

  const glm::ivec2 kFaceOffsets[] = {
      glm::ivec2(faceSize, faceSize * 3),
      glm::ivec2(0, faceSize),
      glm::ivec2(faceSize, faceSize),
      glm::ivec2(faceSize * 2, faceSize),
      glm::ivec2(faceSize, 0),
      glm::ivec2(faceSize, faceSize * 2),
  };

  const int clampW = b.width() - 1;
  const int clampH = b.height() - 1;

  for (int face = 0; face != 6; face++)
  {
    for (int i = 0; i != faceSize; i++)
    {
      for (int j = 0; j != faceSize; j++)
      {
        const glm::vec3 P     = face_coords_to_xyz(i, j, face, faceSize);
        const float     R     = hypot(P.x, P.y);
        const float     theta = atan2(P.y, P.x);
        const float     phi   = atan2(P.z, R);
        //	float point source coordinates
        const float Uf = float(2.0f * faceSize * (theta + M_PI) / M_PI);
        const float Vf = float(2.0f * faceSize * (M_PI / 2.0f - phi) / M_PI);
        // 4-samples for bilinear interpolation
        const int U1 = glm::clamp(int(floor(Uf)), 0, clampW);
        const int V1 = glm::clamp(int(floor(Vf)), 0, clampH);
        const int U2 = glm::clamp(U1 + 1, 0, clampW);
        const int V2 = glm::clamp(V1 + 1, 0, clampH);
        // fractional part
        const float s = Uf - U1;
        const float t = Vf - V1;
        // fetch 4-samples
        const glm::vec4 A = b.pixel(U1, V1);
        const glm::vec4 B = b.pixel(U2, V1);
        const glm::vec4 C = b.pixel(U1, V2);
        const glm::vec4 D = b.pixel(U2, V2);
        // bilinear interpolation
        const glm::vec4 color = A * (1 - s) * (1 - t) + B * (s) * (1 - t) +
                                C * (1 - s) * t + D * (s) * (t);
        result.set_pixel(i + kFaceOffsets[face].x,
                         j + kFaceOffsets[face].y,
                         color);
      }
    };
  }

  return result;
}

Image convert_vertical_cross_to_cube_map_faces(const Image &b)
{
  const int faceWidth  = b.width() / 3;
  const int faceHeight = b.height() / 4;

  Image cubemap(faceWidth, faceHeight, 6, b.channels_count(), b.format());

  const uint8_t *src = b.data();
  uint8_t       *dst = cubemap.data();

  /*
                  ------
                  | +Y |
   ----------------
   | -X | -Z | +X |
   ----------------
                  | -Y |
                  ------
                  | +Z |
                  ------
  */

  const int pixelSize =
      cubemap.channels_count() * cubemap.bytes_per_component();

  for (int face = 0; face != 6; ++face)
  {
    for (int j = 0; j != faceHeight; ++j)
    {
      for (int i = 0; i != faceWidth; ++i)
      {
        int x = 0;
        int y = 0;

        switch (face)
        {
          // GL_TEXTURE_CUBE_MAP_POSITIVE_X
        case 0:
          x = i;
          y = faceHeight + j;
          break;

          // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
        case 1:
          x = 2 * faceWidth + i;
          y = 1 * faceHeight + j;
          break;

          // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
        case 2:
          x = 2 * faceWidth - (i + 1);
          y = 1 * faceHeight - (j + 1);
          break;

          // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
        case 3:
          x = 2 * faceWidth - (i + 1);
          y = 3 * faceHeight - (j + 1);
          break;

          // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
        case 4:
          x = 2 * faceWidth - (i + 1);
          y = b.height() - (j + 1);
          break;

          // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
        case 5:
          x = faceWidth + i;
          y = faceHeight + j;
          break;
        }

        memcpy(dst, src + (y * b.width() + x) * pixelSize, pixelSize);

        dst += pixelSize;
      }
    }
  }

  return cubemap;
}

Image::Image(int width, int height, int channels_count, ImageFormat format)
    : width_{width},
      height_{height},
      channels_count_{channels_count},
      format_{format}
{
  const auto size = width_ * height_ * channels_count_ * bytes_per_component();
  data_.resize(size);
}

Image::Image(int         width,
             int         height,
             int         depth,
             int         channels_count,
             ImageFormat format)
    : width_{width},
      height_{height},
      channels_count_{channels_count},
      format_{format},
      type_{ImageType::Cube}
{
  const auto size =
      width_ * height_ * depth * channels_count_ * bytes_per_component();
  data_.resize(size);
}

Image::Image(int           width,
             int           height,
             int           channels_count,
             ImageFormat   format,
             std::uint8_t *data)
    : width_{width},
      height_{height},
      channels_count_{channels_count},
      format_{format}
{
  const auto size = width_ * height_ * channels_count_ * bytes_per_component();
  data_.resize(size);
  std::memcpy(data_.data(), data, size);
}

Image::Image(const std::filesystem::path &file_path)
{
  load_from_file(file_path);
}

Image::Image(Image &&other)
    : width_{other.width_},
      height_{other.height_},
      channels_count_{other.channels_count_},
      format_{other.format_},
      data_{std::move(other.data_)},
      type_{other.type_}
{
}

int Image::width() const { return width_; }

int Image::height() const { return height_; }

int Image::channels_count() const { return channels_count_; }

ImageFormat Image::format() const { return format_; }

bool Image::is_euqirectangular() const { return width_ == 2 * height_; }

ImageType Image::type() const { return type_; }

void Image::set_type(ImageType type) { type_ = type; }

unsigned char const *Image::data() const { return data_.data(); }

unsigned char *Image::data() { return data_.data(); }

void Image::load_from_file(const std::filesystem::path &file_path)
{
  stbi_set_flip_vertically_on_load(true);
  const auto data = stbi_load(file_path.string().c_str(),
                              &width_,
                              &height_,
                              &channels_count_,
                              0);
  if (!data)
  {
    throw std::runtime_error("Could not load image " + file_path.string());
  }

  const auto size = width_ * height_ * channels_count_;
  data_.resize(size);
  std::memcpy(data_.data(), data, size);
  stbi_image_free(data);
}

void Image::set_pixel(int x, int y, const glm::vec4 &color)
{
  switch (format_)
  {
  case ImageFormat::Float:
  {
    const auto ofs  = channels_count_ * (y * width_ + x);
    auto       data = reinterpret_cast<float *>(data_.data());
    if (channels_count_ > 0)
    {
      data[ofs + 0] = color.x;
    }
    if (channels_count_ > 1)
    {
      data[ofs + 1] = color.y;
    }
    if (channels_count_ > 2)
    {
      data[ofs + 2] = color.z;
    }
    if (channels_count_ > 3)
    {
      data[ofs + 3] = color.w;
    }
  }
  break;
  case ImageFormat::UnsignedByte:
  {
    const auto ofs = channels_count_ * (y * width_ + x);
    if (channels_count_ > 0)
    {
      data_[ofs + 0] = std::uint8_t(color.x * 255.0f);
    }
    if (channels_count_ > 1)
    {
      data_[ofs + 1] = std::uint8_t(color.y * 255.0f);
    }
    if (channels_count_ > 2)
    {
      data_[ofs + 2] = std::uint8_t(color.z * 255.0f);
    }
    if (channels_count_ > 3)
    {
      data_[ofs + 3] = std::uint8_t(color.w * 255.0f);
    }
  }
  break;
  }
}

glm::vec4 Image::pixel(int x, int y) const
{
  switch (format_)
  {
  case ImageFormat::Float:
  {
    const auto ofs  = channels_count_ * (y * width_ + x);
    const auto data = reinterpret_cast<const float *>(data_.data());
    return glm::vec4(channels_count_ > 0 ? data[ofs + 0] : 0.0f,
                     channels_count_ > 1 ? data[ofs + 1] : 0.0f,
                     channels_count_ > 2 ? data[ofs + 2] : 0.0f,
                     channels_count_ > 3 ? data[ofs + 3] : 0.0f);
  }

  case ImageFormat::UnsignedByte:
  {
    const auto ofs = channels_count_ * (y * width_ + x);
    return glm::vec4(
        channels_count_ > 0 ? float(data_[ofs + 0]) / 255.0f : 0.0f,
        channels_count_ > 1 ? float(data_[ofs + 1]) / 255.0f : 0.0f,
        channels_count_ > 2 ? float(data_[ofs + 2]) / 255.0f : 0.0f,
        channels_count_ > 3 ? float(data_[ofs + 3]) / 255.0f : 0.0f);
  }
  }

  assert(0);
  return {};
}

int Image::bytes_per_component() const
{
  return (format_ == ImageFormat::Float ? sizeof(float) : sizeof(std::uint8_t));
}

} // namespace dc
