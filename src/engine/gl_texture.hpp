#pragma once

#include "gl.hpp"

#include <cstdint>
#include <filesystem>

namespace dc
{

struct GlTextureConfig
{
  void *data_{nullptr};
  GLint width_{};
  GLint height_{};

  GLenum format_{GL_RGBA};
  GLenum sized_format_{GL_RGBA8};
  GLuint msaa_{0};

  GLint    max_level_{1000};
  GLint    wrap_s_{GL_REPEAT};
  GLint    wrap_t_{GL_REPEAT};
  GLint    min_filter_{GL_LINEAR_MIPMAP_LINEAR};
  GLint    mag_filter_{GL_LINEAR};
  unsigned generate_mipmaps_{true};

  GLenum type_{GL_UNSIGNED_BYTE};
};

class GlTexture
{
public:
  static std::shared_ptr<GlTexture>
  load_from_file(const std::filesystem::path &file_path);

  GlTexture(const GlTextureConfig &config);
  ~GlTexture();

  GLuint id() const;

  void bind_unit(int unit) const;

  GLenum format() const;
  GLenum sized_format() const;

  GLint width() const;
  GLint height() const;

  GLuint mipmap_levels() const;

private:
  GLuint id_{};
  GLenum format_{};
  GLenum sized_format_{};
  GLint  width_{};
  GLint  height_{};
  GLuint mipmap_levels_{0};

  GlTexture(const GlTexture &) = delete;
  void operator=(const GlTexture &) = delete;
  GlTexture(GlTexture &&)           = delete;
  void operator=(GlTexture &&) = delete;
};

} // namespace dc
