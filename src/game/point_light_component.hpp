#pragma once

#include "math.hpp"
#include "gl_cube_texture.hpp"

namespace dc
{

struct PointLightComponent
{
  glm::vec3 color_{1.0f};
  float     multiplier_{1.0f};

  float radius_{10.0f};
  float falloff_{0.2f};

  void set_cast_shadow(bool value);
  bool cast_shadow() const;

  std::shared_ptr<GlCubeTexture> shadow_tex() const;

  bool                           cast_shadow_{false};
  std::shared_ptr<GlCubeTexture> shadow_tex_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
