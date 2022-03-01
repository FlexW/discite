#pragma once

#include "gl_cube_texture.hpp"

#include <filesystem>

class Sky
{
public:
  Sky() = default;
  Sky(const std::filesystem::path &file_path);

  std::shared_ptr<GlCubeTexture> env_texture() const;
  std::shared_ptr<GlCubeTexture> env_irradiance_texture() const;

private:
  std::shared_ptr<GlCubeTexture> env_texture_{};
  std::shared_ptr<GlCubeTexture> env_irradiance_texture_{};
};
