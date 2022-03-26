#pragma once

#include "event.hpp"
#include "game_layer.hpp"
#include "layer.hpp"
#include "viewport_panel.hpp"
#include "window.hpp"

#include <memory>

namespace dc
{

class EditorLayer : public Layer
{
public:
  void init() override;
  void shutdown() override;

  bool update(float delta_time) override;
  bool render() override;

  bool on_event(const Event &event) override;

private:
  bool is_playing_{false};

  void setup_game();
  void set_capture_mouse(bool value);
  bool on_key_event(const KeyEvent &event);
  bool on_play_scene_event(const PlaySceneEvent &event);
};

} // namespace dc
