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
};

} // namespace dc
