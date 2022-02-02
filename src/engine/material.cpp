#include "material.hpp"

void Material::set_diffuse_texture(std::shared_ptr<GlTexture> value)
{
  diffuse_texture_ = value;

  set_transparent(diffuse_texture_->format() == GL_RGBA);
}

std::shared_ptr<GlTexture> Material::diffuse_texture() const
{
  return diffuse_texture_;
}

glm::vec3 Material::diffuse_color() const { return diffuse_color_; }

void Material::set_specular_power(float value) { specular_power_ = value; }

float Material::specular_power() const { return specular_power_; }

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
