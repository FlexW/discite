#include "discite.hpp"
#include "engine/engine.hpp"

#include <memory>

int main()
{
  const auto engine = Engine::instance();
  engine->set_game(std::make_unique<Discite>());
  return engine->run();
}
