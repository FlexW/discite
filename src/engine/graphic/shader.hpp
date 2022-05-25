#pragma once

#include "cube_texture.hpp"
#include "math.hpp"
#include "texture.hpp"

#include <string>
#include <vector>

namespace dc
{

class Shader
{
public:
  virtual ~Shader() = default;

  virtual void set_uniform(const std::string &name, bool value)             = 0;
  virtual void set_uniform(const std::string &name, int value)              = 0;
  virtual void set_uniform(const std::string &name, float value)            = 0;
  virtual void set_uniform(const std::string &name, const glm::vec3 &value) = 0;
  virtual void set_uniform(const std::string &name, const glm::vec4 &value) = 0;
  virtual void set_uniform(const std::string &name, const glm::mat2 &value) = 0;
  virtual void set_uniform(const std::string &name, const glm::mat4 &value) = 0;
  virtual void set_uniform(const std::string            &name,
                           const std::vector<glm::mat4> &value)             = 0;
  virtual void set_uniform(const std::string        &name,
                           const std::vector<float> &value)                 = 0;
  virtual void set_uniform(const std::string      &name,
                           const std::vector<int> &value)                   = 0;
  virtual void set_uniform(const std::string &name,
                           const Texture     &texture,
                           unsigned           count)                                  = 0;

  virtual void set_uniform(const std::string &name,
                           const CubeTexture &texture,
                           unsigned           slot) = 0;
};

} // namespace dc
