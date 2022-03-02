#include "environment_map.hpp"

EnvionmentMap::EnvionmentMap(
    std::shared_ptr<GlCubeTexture> env_texture,
    std::shared_ptr<GlCubeTexture> env_irradiance_texture)
    : env_texture_{env_texture},
      env_irradiance_texture_{env_irradiance_texture}
{
}

std::shared_ptr<GlCubeTexture> EnvionmentMap::env_texture() const
{
  return env_texture_;
}

std::shared_ptr<GlCubeTexture> EnvionmentMap::env_irradiance_texture() const
{
  return env_irradiance_texture_;
}
