#pragma once

#include "gl.hpp"
#include "graphic/texture.hpp"

#include <cstdint>
#include <filesystem>

namespace dc
{

class GlTexture : public Texture
{
public:
  GlTexture(const TextureConfig &config);
  ~GlTexture();

  GLuint id() const;

  void bind_unit(int unit) const;

  TextureFormat      format() const override;
  TextureSizedFormat sized_format() const override;

  std::size_t width() const override;
  std::size_t height() const override;

  unsigned   mipmap_levels() const override;
  glm::ivec2 mipmap_size(unsigned level) const override;

private:
  GLuint             id_{};
  TextureFormat      format_{};
  TextureSizedFormat sized_format_{};
  std::size_t        width_{};
  std::size_t        height_{};
  GLuint             mipmap_levels_{0};

  GlTexture(const GlTexture &) = delete;
  void operator=(const GlTexture &) = delete;
  GlTexture(GlTexture &&)           = delete;
  void operator=(GlTexture &&) = delete;
};

} // namespace dc
