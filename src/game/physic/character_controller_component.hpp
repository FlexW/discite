#pragma once

#include "character_controller.hpp"

namespace dc
{

struct CharacterControllerComponent
{
  float slope_limit_degree_;
  float step_offset_;

  bool disable_gravity_{false};

  CharacterController *controller_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
