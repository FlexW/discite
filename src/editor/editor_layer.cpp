#include "editor_layer.hpp"
#include "dockspace_panel.hpp"
#include "engine.hpp"
#include "imgui_layer.hpp"
#include "renderer_panel.hpp"
#include "viewport_panel.hpp"
#include "window.hpp"

#include <cassert>
#include <memory>

void EditorLayer::init()
{
  setup_game();
  set_capture_mouse(false);

  const auto renderer_panel = std::make_shared<RendererPanel>();
  renderer_panel->set_renderer(game_->renderer());

  const auto viewport_panel = std::make_shared<ViewportPanel>();

  const auto dockspace_panel = std::make_shared<DockspacePanel>();

  const auto imgui_layer =
      Engine::instance()->layer_stack()->layer<ImGuiLayer>();
  imgui_layer->add_panel(dockspace_panel);
  imgui_layer->add_panel(renderer_panel);
  imgui_layer->add_panel(viewport_panel);
}

void EditorLayer::shutdown() {}

void EditorLayer::update(float /*delta_time*/) {}

void EditorLayer::render() { game_->render(); }

void EditorLayer::setup_game()
{
  assert(!game_);

  game_ = std::make_unique<Discite>();
  game_->init();
}

void EditorLayer::set_capture_mouse(bool value)
{
  const auto window = Engine::instance()->window();
  window->set_capture_mouse(value);
}

bool EditorLayer::on_event(const Event &event)
{
  const auto event_id = event.id();
  if (event_id == KeyEvent::id)
  {
    return on_key_event(dynamic_cast<const KeyEvent &>(event));
  }

  return false;
}

bool EditorLayer::on_key_event(const KeyEvent &event)
{
  if (event.key_ == Key::Escape)
  {
    Engine::instance()->set_close(true);
  }

  return false;
}
