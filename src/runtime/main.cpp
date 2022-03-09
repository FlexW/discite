#include "engine.hpp"
#include "game_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = dc::Engine::instance();
  engine->push_layer(std::make_unique<dc::GameLayer>());
  return engine->run(argc, argv);
}
