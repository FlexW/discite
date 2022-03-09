#pragma once

#include "math.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace dc
{

enum class ImageType
{
  TwoD,
  Cube,
};

enum class ImageFormat
{
  UnsignedByte,
  Float,
};

class Image
{
public:
  Image() = default;
  Image(int width, int height, int channels_count, ImageFormat format);
  Image(int         width,
        int         height,
        int         depth,
        int         channels_count,
        ImageFormat format);
  Image(int           width,
        int           height,
        int           channels_count,
        ImageFormat   format,
        std::uint8_t *data);
  Image(const std::filesystem::path &file_path);
  ~Image() = default;

  Image(Image &&other);

  int width() const;
  int height() const;
  int channels_count() const;

  bool        is_euqirectangular() const;
  ImageFormat format() const;

  void        set_type(ImageType type);
  ImageType   type() const;

  unsigned char const *data() const;
  unsigned char       *data();

  void load_from_file(const std::filesystem::path &file_path);

  void      set_pixel(int x, int y, const glm::vec4 &color);
  glm::vec4 pixel(int x, int y) const;

  int bytes_per_component() const;

private:
  int                       width_{};
  int                       height_{};
  int                       channels_count_{};
  ImageFormat               format_{ImageFormat::UnsignedByte};
  std::vector<std::uint8_t> data_{};
  ImageType                 type_{ImageType::TwoD};

  Image(const Image &other) = delete;
  void operator=(const Image &other) = delete;
};

Image convert_equirectangular_map_to_vertical_cross(const Image &b);
Image convert_vertical_cross_to_cube_map_faces(const Image &b);

} // namespace dc
