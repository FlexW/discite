#pragma once

#include "math.hpp"

#include <cstdint>

namespace dc
{

enum class BroadphaseType
{
  SweepAndPrune,
  MultiBoxPrune,
  AutomaticBoxPrune
};

enum class FrictionType
{
  Patch,
  OneDirectional,
  TwoDirectional
};

} // namespace dc
