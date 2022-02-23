#include "material.hpp"

void Material::set_albedo_texture(std::shared_ptr<GlTexture> value)
{
  albedo_texture_ = value;

  if (albedo_texture_)
  {
    set_transparent(albedo_texture_->format() == GL_RGBA);
  }
}

std::shared_ptr<GlTexture> Material::albedo_texture() const
{
  return albedo_texture_;
}

glm::vec3 Material::albedo_color() const { return albedo_color_; }

void Material::set_normal_texture(std::shared_ptr<GlTexture> value)
{
  normal_texture_ = value;
}

std::shared_ptr<GlTexture> Material::normal_texture() const
{
  return normal_texture_;
}

void Material::set_transparent(bool value) { is_transparent_ = value; }

bool Material::is_transparent() const { return is_transparent_; }

void Material::set_roughness_texture(std::shared_ptr<GlTexture> value)
{
  roughness_texture_ = value;
}

std::shared_ptr<GlTexture> Material::roughness_texture() const
{
  return roughness_texture_;
}

void Material::set_roughness(float value) { roughess_ = value; }

float Material::roughness() const { return roughess_; }

void Material::set_ambient_occlusion_texture(std::shared_ptr<GlTexture> value)
{
  ambient_occlusion_texture_ = value;
}

std::shared_ptr<GlTexture> Material::ambient_occlusion_texture() const
{
  return ambient_occlusion_texture_;
}

void Material::set_emissive_texture(std::shared_ptr<GlTexture> value)
{
  emissive_texture_ = value;
}

std::shared_ptr<GlTexture> Material::emissive_texture() const
{
  return emissive_texture_;
}
