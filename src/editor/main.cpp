#include "editor_layer.hpp"
#include "engine.hpp"
#include "game_layer.hpp"
#include "imgui_layer.hpp"

#include <memory>

int main(int argc, char *argv[])
{
  const auto engine = dc::Engine::instance();
  engine->push_layer(std::make_unique<dc::EditorLayer>());
  engine->push_layer(std::make_unique<dc::ImGuiLayer>());
  engine->push_layer(std::make_unique<dc::GameLayer>());
  return engine->run(argc, argv);
}
