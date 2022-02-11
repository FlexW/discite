#include "editor_layer.hpp"
#include "engine.hpp"
#include "imgui_layer.hpp"
#include "renderer_panel.hpp"
#include "window.hpp"

#include <cassert>
#include <memory>

void EditorLayer::init()
{
  setup_game();
  set_capture_mouse(false);

  const auto renderer_panel = std::make_shared<RendererPanel>();
  renderer_panel->set_renderer(game_->renderer());
  const auto imgui_layer =
      Engine::instance()->layer_stack()->layer<ImGuiLayer>();
  imgui_layer->add_panel(renderer_panel);
}

void EditorLayer::shutdown() {}

void EditorLayer::update(float delta_time)
{
  if (is_play_game_)
  {
    game_->update(delta_time);
  }
}

void EditorLayer::render() { game_->render(); }

void EditorLayer::start_play_game()
{
  assert(game_);
  is_play_game_ = true;
  set_capture_mouse(true);
}

void EditorLayer::setup_game()
{
  assert(!game_);

  game_ = std::make_unique<Discite>();
  game_->init();
}

void EditorLayer::stop_play_game()
{
  is_play_game_ = false;

  set_capture_mouse(false);
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

  if (is_play_game_)
  {
    assert(game_);
    game_->on_event(event);

    // prevent other layers (imgui layer) from processing events
    return true;
  }

  return false;
}

bool EditorLayer::on_key_event(const KeyEvent &event)
{
  if (event.key_ == Key::LeftControl && event.key_action_ == KeyAction::Press)
  {
    start_play_game();
  }
  else if (event.key_ == Key::LeftControl &&
           event.key_action_ == KeyAction::Release)
  {
    stop_play_game();
  }

  if (event.key_ == Key::Escape)
  {
    Engine::instance()->set_close(true);
  }

  return false;
}
