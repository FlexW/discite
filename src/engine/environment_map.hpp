#pragma once

#include "gl_cube_texture.hpp"

namespace dc
{

class EnvironmentMap
{
public:
  EnvironmentMap() = default;
  EnvironmentMap(const std::string &name, std::vector<std::uint8_t> data);

  std::string name() const;

  std::vector<std::uint8_t> data() const;

  void release_data();

private:
  std::string               name_;
  std::vector<std::uint8_t> data_{};
};

} // namespace dc
