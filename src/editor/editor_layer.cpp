#include "editor_layer.hpp"
#include "dockspace_panel.hpp"
#include "engine.hpp"
#include "entity_panel.hpp"
#include "game_layer.hpp"
#include "imgui_layer.hpp"
#include "performance_panel.hpp"
#include "profiling.hpp"
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

  const auto game_layer = Engine::instance()->layer_stack()->layer<GameLayer>();
  DC_ASSERT(game_layer, "Game layer not loaded");

  const auto renderer_panel = std::make_shared<RendererPanel>();
  const auto viewport_panel = std::make_shared<ViewportPanel>();

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

bool EditorLayer::update(float delta_time)
{
  if (is_playing_)
  {
    if (const auto game_layer =
            Engine::instance()->layer_stack()->layer<GameLayer>();
        game_layer)
    {
      game_layer->update(delta_time);
    }
  }
  return false;
}

bool EditorLayer::render() { return false; }

void EditorLayer::setup_game()
{
  const auto game_layer = Engine::instance()->layer_stack()->layer<GameLayer>();
  // Init if not already done
  game_layer->init();

  DC_ASSERT(game_layer, "Game layer not loaded");
  if (game_layer)
  {
    game_layer->set_scene(std::dynamic_pointer_cast<SceneAssetHandle>(
        Engine::instance()->asset_cache()->load_asset(
            Asset{"scenes/sponza.dcscn"})));
  }
}

void EditorLayer::set_capture_mouse(bool value)
{
  const auto window = Engine::instance()->window();
  window->set_capture_mouse(value);
}

bool EditorLayer::on_event(const Event &event)
{
  DC_PROFILE_SCOPE("EditorLayer::on_event()");

  const auto event_id = event.id();
  if (event_id == KeyEvent::id)
  {
    return on_key_event(dynamic_cast<const KeyEvent &>(event));
  }
  else if (event_id == PlaySceneEvent::id)
  {
    return on_play_scene_event(dynamic_cast<const PlaySceneEvent &>(event));
  }

  return false;
}

bool EditorLayer::on_key_event(const KeyEvent &event)
{
  DC_PROFILE_SCOPE("EditorLayer::on_key_event()");

  if (event.key_ == Key::S && event.ctrl_pressed_)
  {
    const auto game_layer =
        Engine::instance()->layer_stack()->layer<GameLayer>();
    DC_ASSERT(game_layer, "Game layer not loaded");
    const auto scene = game_layer->scene();
    if (scene && scene->is_ready())
    {
      scene->save();
    }
  }

  return false;
}

bool EditorLayer::on_play_scene_event(const PlaySceneEvent &event)
{
  is_playing_ = event.play_;

  return false;
}

} // namespace dc
