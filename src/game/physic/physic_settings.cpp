#include "physic_settings.hpp"

namespace dc
{

PhysicsSettings default_physic_settings()
{
  static PhysicsSettings settings{};
  return settings;
}

} // namespace dc
