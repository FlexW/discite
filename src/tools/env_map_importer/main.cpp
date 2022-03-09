#include "engine.hpp"
#include "env_map_importer_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = dc::Engine::instance();
  engine->push_layer(std::make_unique<dc::EnvMapImporter>());
  return engine->run(argc, argv, false);
}
