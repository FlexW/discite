#pragma once

#include "gl_texture.hpp"
#include "math.hpp"

#include <memory>

class Material
{
public:
  void set_diffuse_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> diffuse_texture() const;

  void      set_diffuse_color(const glm::vec3 &value);
  glm::vec3 diffuse_color() const;

  void  set_specular_power(float value);
  float specular_power() const;

  void set_normal_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> normal_texture() const;

  void set_transparent(bool value);
  bool is_transparent() const;

  void set_selected(bool value);
  bool is_selected() const;

private:
  float specular_power_{200.0f};

  std::shared_ptr<GlTexture> diffuse_texture_{};

  glm::vec3 diffuse_color_{0.6f};

  std::shared_ptr<GlTexture> normal_texture_{};

  bool is_transparent_{false};
  bool is_selected_{false};
};
