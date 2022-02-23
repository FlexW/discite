#pragma once

#include "gl_texture.hpp"
#include "math.hpp"

#include <memory>

class Material
{
public:
  void set_albedo_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> albedo_texture() const;

  void      set_albedo_color(const glm::vec3 &value);
  glm::vec3 albedo_color() const;

  void set_roughness_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> roughness_texture() const;

  void  set_roughness(float value);
  float roughness() const;

  void set_ambient_occlusion_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> ambient_occlusion_texture() const;

  void set_emissive_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> emissive_texture() const;

  void set_normal_texture(std::shared_ptr<GlTexture> value);
  std::shared_ptr<GlTexture> normal_texture() const;

  void set_transparent(bool value);
  bool is_transparent() const;

private:
  std::shared_ptr<GlTexture> albedo_texture_{};
  glm::vec3                  albedo_color_{0.6f};

  std::shared_ptr<GlTexture> roughness_texture_{};
  float                      roughess_{0.6f};

  std::shared_ptr<GlTexture> ambient_occlusion_texture_{};
  std::shared_ptr<GlTexture> emissive_texture_{};
  std::shared_ptr<GlTexture> normal_texture_{};

  bool is_transparent_{false};
};
