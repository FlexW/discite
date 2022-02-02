#pragma once

#include "math.hpp"

class PointLight
{
public:
  void      set_position(const glm::vec3 &value);
  glm::vec3 position() const;

  void      set_ambient_color(const glm::vec3 &value);
  glm::vec3 ambient_color();

  void      set_diffuse_color(const glm::vec3 &value);
  glm::vec3 diffuse_color();

  void      set_specular_color(const glm::vec3 &value);
  glm::vec3 specular_color();

  void  set_constant(float value);
  float constant() const;

  void  set_linear(float value);
  float linear() const;

  void  set_quadratic(float value);
  float quadratic() const;

private:
  glm::vec3 position_{0.0f};

  glm::vec3 ambient_color_{5.0f};
  glm::vec3 diffuse_color_{10.0f};
  glm::vec3 specular_color_{20.0f};

  float constant_{1.0f};
  float linear_{0.9f};
  float quadratic_{1.8f};
};
