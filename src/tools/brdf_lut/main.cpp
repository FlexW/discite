#include "brdf_lut_layer.hpp"
#include "engine.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = dc::Engine::instance();
  engine->push_layer(std::make_unique<dc::BrdfLutLayer>());
  return engine->run(argc, argv, false);
}
