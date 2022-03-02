#pragma once

#include "asset.hpp"
#include "gl_texture.hpp"
#include "math.hpp"
#include "texture_asset.hpp"

#include <memory>

class Material
{
public:
  void set_albedo_texture(std::shared_ptr<TextureAssetHandle> value);
  std::shared_ptr<GlTexture> albedo_texture() const;

  void      set_albedo_color(const glm::vec4 &value);
  glm::vec4 albedo_color() const;

  void set_roughness_texture(std::shared_ptr<TextureAssetHandle> value);
  std::shared_ptr<GlTexture> roughness_texture() const;

  void  set_roughness(const glm::vec4 &value);
  glm::vec4 roughness() const;

  void set_ambient_occlusion_texture(std::shared_ptr<TextureAssetHandle> value);
  std::shared_ptr<GlTexture> ambient_occlusion_texture() const;

  void set_emissive_texture(std::shared_ptr<TextureAssetHandle> value);
  std::shared_ptr<GlTexture> emissive_texture() const;

  void      set_emissive_color(const glm::vec4 &value);
  glm::vec4 emissive_color() const;

  void set_normal_texture(std::shared_ptr<TextureAssetHandle> value);
  std::shared_ptr<GlTexture> normal_texture() const;

  void set_transparent(bool value);
  bool is_transparent() const;

private:
  std::shared_ptr<TextureAssetHandle> albedo_texture_{};
  glm::vec4                           albedo_color_{0.6f};

  std::shared_ptr<TextureAssetHandle> roughness_texture_{};
  glm::vec4                           roughess_{0.6f};

  std::shared_ptr<TextureAssetHandle> emissive_texture_{};
  glm::vec4                           emissive_color_;

  std::shared_ptr<TextureAssetHandle> ambient_occlusion_texture_{};

  std::shared_ptr<TextureAssetHandle> normal_texture_{};

  bool is_transparent_{false};
};
