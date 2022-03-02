#pragma once

#include "gl_cube_texture.hpp"

class EnvionmentMap
{
public:
  EnvionmentMap() = default;
  EnvionmentMap(std::shared_ptr<GlCubeTexture> env_texture,
                std::shared_ptr<GlCubeTexture> env_irradiance_texture);

  std::shared_ptr<GlCubeTexture> env_texture() const;
  std::shared_ptr<GlCubeTexture> env_irradiance_texture() const;

private:
  std::shared_ptr<GlCubeTexture> env_texture_{};
  std::shared_ptr<GlCubeTexture> env_irradiance_texture_{};
};
