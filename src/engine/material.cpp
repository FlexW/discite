#include "material.hpp"

void Material::set_ambient_texture(std::shared_ptr<GlTexture> value)
{
  ambient_texture_ = value;
}

std::shared_ptr<GlTexture> Material::ambient_texture() const
{
  return ambient_texture_;
}

void Material::set_diffuse_texture(std::shared_ptr<GlTexture> value)
{
  diffuse_texture_ = value;
}
std::shared_ptr<GlTexture> Material::diffuse_texture() const
{
  return diffuse_texture_;
}

void Material::set_specular_texture(std::shared_ptr<GlTexture> value)
{
  specular_texture_ = value;
}

std::shared_ptr<GlTexture> Material::specular_texture() const
{
  return specular_texture_;
}

void Material::set_ambient_color(const glm::vec3 &value)
{
  ambient_color_ = value;
}

glm::vec3 Material::ambient_color() const { return ambient_color_; }

void Material::set_diffuse_color(const glm::vec3 &value)
{
  diffuse_color_ = value;
}

glm::vec3 Material::diffuse_color() const { return diffuse_color_; }

void Material::set_specular_color(const glm::vec3 &value)
{
  specular_color_ = value;
}

glm::vec3 Material::specular_color() const { return specular_color_; }

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
