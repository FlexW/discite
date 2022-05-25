#pragma once

#include "gl.hpp"
#include "graphic/cube_texture.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace dc
{

class GlCubeTexture : public CubeTexture
{
public:
  explicit GlCubeTexture(const CubeTextureConfig &config);
  // explicit GlCubeTexture(const std::vector<std::uint8_t> &hdr_data);
  ~GlCubeTexture();

  GLuint id() const;

  void bind_unit(int unit) const;

  unsigned mipmap_levels() const override;

private:
  GLuint id_{};
  GLuint mipmap_levels_{};
};

} // namespace dc
