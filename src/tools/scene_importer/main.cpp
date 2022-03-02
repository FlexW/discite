#include "engine.hpp"
#include "scene_importer_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = Engine::instance();
  engine->push_layer(std::make_unique<SceneImporterLayer>());
  return engine->run(argc, argv, false);
}
