#pragma once

#include "entity.hpp"

namespace dc
{

class PhysicController
{
public:
  PhysicController(Entity entity);

  Entity entity() const;

  void sync_transform();
};

} // namespace dc
