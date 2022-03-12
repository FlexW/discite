#include "editor_layer.hpp"
#include "dockspace_panel.hpp"
#include "engine.hpp"
#include "entity_panel.hpp"
#include "imgui_layer.hpp"
#include "performance_panel.hpp"
#include "renderer_panel.hpp"
#include "scene_asset.hpp"
#include "scene_panel.hpp"
#include "serialization.hpp"
#include "viewport_panel.hpp"
#include "window.hpp"

#include <memory>

namespace dc
{

void EditorLayer::init()
{
  setup_game();
  set_capture_mouse(false);

  const auto dockspace_panel = std::make_shared<DockspacePanel>();

  const auto renderer_panel = std::make_shared<RendererPanel>();
  renderer_panel->set_renderer(game_layer_->renderer());

  const auto viewport_panel = std::make_shared<ViewportPanel>();
  viewport_panel->set_renderer(game_layer_->renderer());

  const auto scene_panel       = std::make_shared<ScenePanel>();
  const auto entity_panel      = std::make_shared<EntityPanel>();
  const auto performance_panel = std::make_shared<PerformancePanel>();

  const auto imgui_layer =
      Engine::instance()->layer_stack()->layer<ImGuiLayer>();
  imgui_layer->add_panel(dockspace_panel);
  imgui_layer->add_panel(renderer_panel);
  imgui_layer->add_panel(viewport_panel);
  imgui_layer->add_panel(scene_panel);
  imgui_layer->add_panel(entity_panel);
  imgui_layer->add_panel(performance_panel);
}

void EditorLayer::shutdown() {}

void EditorLayer::update(float /*delta_time*/) {}

void EditorLayer::render() {}

void EditorLayer::setup_game()
{
  assert(!game_layer_);

  game_layer_ = std::make_unique<GameLayer>();
  game_layer_->register_asset_loaders();
  // TODO: Set a default scene
  game_layer_->set_scene(std::dynamic_pointer_cast<SceneAssetHandle>(
      Engine::instance()->asset_cache()->load_asset(
          Asset{"scenes/sponza.dcscn"})));
  game_layer_->init();
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
  if (event.key_ == Key::S && event.ctrl_pressed_)
  {
    const auto scene = game_layer_->scene();
    if (scene && scene->is_ready())
    {
      scene->save();
    }
  }

  return false;
}

} // namespace dc
