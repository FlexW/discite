#include "engine.hpp"
#include "game_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = Engine::instance();
  engine->push_layer(std::make_unique<GameLayer>());
  return engine->run(argc, argv);
}
