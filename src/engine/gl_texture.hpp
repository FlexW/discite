#pragma once

#include "gl.hpp"

#include <filesystem>

namespace dc
{

class GlTexture
{
public:
  GlTexture();
  ~GlTexture();

  GLuint id() const;

  void load_from_file(const std::filesystem::path &file_path,
                      bool                         generate_mipmap);
  void set_data(const std::uint8_t *data,
                int                 width,
                int                 height,
                int                 channels_count,
                bool                generate_mipmap);
  void set_storage(GLsizei width,
                   GLsizei height,
                   GLint   internal_format,
                   GLenum  format);

  void bind();
  void unbind();

  GLenum format() const;

private:
  GLuint id_{};

  GLenum format_{GL_RGB};

  GlTexture(const GlTexture &) = delete;
  void operator=(const GlTexture &) = delete;
  GlTexture(GlTexture &&)           = delete;
  void operator=(GlTexture &&) = delete;
};

} // namespace dc
