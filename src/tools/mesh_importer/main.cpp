#include "engine.hpp"
#include "mesh_importer_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = Engine::instance();
  engine->push_layer(std::make_unique<MeshImporterLayer>());
  return engine->run(argc, argv, false);
}
