#include "material.hpp"
#include "texture_asset.hpp"

namespace dc
{

void Material::set_albedo_texture(std::shared_ptr<TextureAssetHandle> value)
{
  albedo_texture_ = value;
}

std::shared_ptr<GlTexture> Material::albedo_texture() const
{
  if (!albedo_texture_ || !albedo_texture_->is_ready())
  {
    return nullptr;
  }
  return albedo_texture_->get();
}

void Material::set_albedo_color(const glm::vec4 &value)
{
  albedo_color_ = value;
}

glm::vec4 Material::albedo_color() const { return albedo_color_; }

void Material::set_normal_texture(std::shared_ptr<TextureAssetHandle> value)
{
  normal_texture_ = value;
}

std::shared_ptr<GlTexture> Material::normal_texture() const
{
  if (!normal_texture_ || !normal_texture_->is_ready())
  {
    return nullptr;
  }
  return normal_texture_->get();
}

void Material::set_transparent(bool value) { is_transparent_ = value; }

bool Material::is_transparent() const { return is_transparent_; }

void Material::set_roughness_texture(std::shared_ptr<TextureAssetHandle> value)
{
  roughness_texture_ = value;
}

std::shared_ptr<GlTexture> Material::roughness_texture() const
{
  if (!roughness_texture_ || !roughness_texture_->is_ready())
  {
    return nullptr;
  }
  return roughness_texture_->get();
}

void Material::set_roughness(const glm::vec4 &value) { roughess_ = value; }

glm::vec4 Material::roughness() const { return roughess_; }

void Material::set_ambient_occlusion_texture(
    std::shared_ptr<TextureAssetHandle> value)
{
  ambient_occlusion_texture_ = value;
}

std::shared_ptr<GlTexture> Material::ambient_occlusion_texture() const
{
  if (!ambient_occlusion_texture_ || !ambient_occlusion_texture_->is_ready())
  {
    return nullptr;
  }
  return ambient_occlusion_texture_->get();
}

void Material::set_emissive_texture(std::shared_ptr<TextureAssetHandle> value)
{
  emissive_texture_ = value;
}

std::shared_ptr<GlTexture> Material::emissive_texture() const
{
  if (!emissive_texture_ || !emissive_texture_->is_ready())
  {
    return nullptr;
  }
  return emissive_texture_->get();
}

void Material::set_emissive_color(const glm::vec4 &value)
{
  emissive_color_ = value;
}
glm::vec4 Material::emissive_color() const { return emissive_color_; }

} // namespace dc
