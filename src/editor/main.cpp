#include "editor_layer.hpp"
#include "engine.hpp"
#include "imgui_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = Engine::instance();
  engine->push_layer(std::make_unique<EditorLayer>());
  engine->push_layer(std::make_unique<ImGuiLayer>());
  return engine->run(argc, argv);
}
