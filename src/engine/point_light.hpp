#pragma once

#include "math.hpp"
#include "platform/gl/gl_cube_texture.hpp"

namespace dc
{

class PointLight
{
public:
  static constexpr GLuint shadow_map_size{1024};

  void      set_position(const glm::vec3 &value);
  glm::vec3 position() const;

  void      set_color(const glm::vec3 &value);
  glm::vec3 color() const;

  void  set_multiplier(float value);
  float multiplier() const;

  void  set_radius(float value);
  float radius() const;

  void  set_falloff(float value);
  float falloff() const;

  void set_cast_shadow(bool value);
  bool cast_shadow() const;

  void set_shadow_tex(std::shared_ptr<GlCubeTexture> value);
  std::shared_ptr<GlCubeTexture> shadow_tex() const;

private:
  glm::vec3 position_{0.0f};
  glm::vec3 color_{1.0f};
  float     multiplier_{1.0f};

  float radius_{10.0f};
  float falloff_{0.2f};

  std::shared_ptr<GlCubeTexture> shadow_tex_{};
  bool                           cast_shadow_{false};
};

} // namespace dc
