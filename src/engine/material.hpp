#pragma once

#include "gl_texture.hpp"
#include "math.hpp"

#include <memory>

class Material
{
public:
  void set_ambient_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> ambient_texture() const;

  void set_diffuse_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> diffuse_texture() const;

  void set_specular_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> specular_texture() const;

  void      set_ambient_color(const glm::vec3 &value);
  glm::vec3 ambient_color() const;

  void      set_diffuse_color(const glm::vec3 &value);
  glm::vec3 diffuse_color() const;

  void      set_specular_color(const glm::vec3 &value);
  glm::vec3 specular_color() const;

  void  set_specular_power(float value);
  float specular_power() const;

  void set_normal_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> normal_texture() const;

private:
  float specular_power_{200.0f};

  std::shared_ptr<GlTexture> ambient_texture_{};
  std::shared_ptr<GlTexture> diffuse_texture_{};
  std::shared_ptr<GlTexture> specular_texture_{};

  glm::vec3 ambient_color_{0.6f};
  glm::vec3 diffuse_color_{0.6f};
  glm::vec3 specular_color_{0.6f};

  std::shared_ptr<GlTexture> normal_texture_{};
};
