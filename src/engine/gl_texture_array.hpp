#pragma once

#include "gl.hpp"

#include <optional>
#include <vector>

namespace dc
{

struct GlTextureArrayConfig
{
  GlTextureArrayConfig(GLenum sized_format,
                       GLenum format,
                       int    width,
                       int    height,
                       int    count)
      : sized_format{sized_format},

        format{format},
        width{width},
        height{height},
        count{count}
  {
  }

  std::vector<unsigned char *> data{};
  GLenum         sized_format;
  GLenum         format;
  GLenum         type{GL_UNSIGNED_BYTE};
  int            width;
  int            height;
  int            count;

  bool is_generate_mipmap{false};

  GLenum mag_filter{GL_NEAREST};
  GLenum min_filter{GL_NEAREST};
  GLenum wrap_s{GL_CLAMP_TO_BORDER};
  GLenum wrap_t{GL_CLAMP_TO_BORDER};

  std::optional<std::vector<float>> border_color;
};

class GlTextureArray
{
public:
  GlTextureArray(const GlTextureArrayConfig &texture_array_data);
  ~GlTextureArray();

  GLuint id() const;

  void bind_unit(int unit) const;

private:
  GLuint id_{};

  GlTextureArray(const GlTextureArray &) = delete;
  void operator=(const GlTextureArray &) = delete;
  GlTextureArray(GlTextureArray &&)      = delete;
  void operator=(GlTextureArray &&) = delete;
};

} // namespace dc
