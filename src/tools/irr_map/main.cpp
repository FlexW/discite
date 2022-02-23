#include "engine.hpp"
#include "irr_map_layer.hpp"

#include <memory>

int main()
{
  const auto engine = Engine::instance();
  engine->push_layer(std::make_unique<IrrMapLayer>());
  return engine->run(true);
}
