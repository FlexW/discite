#include "discite.hpp"
#include "engine.hpp"

#include <memory>

int main()
{
  const auto engine = Engine::instance();
  engine->push_layer(std::make_unique<Discite>());
  return engine->run();
}
