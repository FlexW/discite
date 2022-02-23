#include "brdf_lut_layer.hpp"
#include "engine.hpp"

#include <memory>

int main()
{
  const auto engine = Engine::instance();
  engine->push_layer(std::make_unique<BrdfLutLayer>());
  return engine->run(true);
}
