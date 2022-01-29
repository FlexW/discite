#pragma once

#include "math.hpp"

class DirectionalLight
{
public:
  void      set_direction(const glm::vec3 &value);
  glm::vec3 direction() const;

  void      set_ambient_color(const glm::vec3 &value);
  glm::vec3 ambient_color() const;

  void      set_diffuse_color(const glm::vec3 &value);
  glm::vec3 diffuse_color() const;

  void      set_specular_color(const glm::vec3 &value);
  glm::vec3 specular_color() const;

private:
  glm::vec3 direction_ = glm::normalize(glm::vec3{0.0f, -1.0f, -1.0f});

  glm::vec3 ambient_color_{0.2f};
  glm::vec3 diffuse_color_{0.8f};
  glm::vec3 specular_color_{1.0f};
};
