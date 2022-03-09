#include "engine.hpp"
#include "mesh_importer_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = dc::Engine::instance();
  engine->push_layer(std::make_unique<dc::MeshImporterLayer>());
  return engine->run(argc, argv, false);
}
