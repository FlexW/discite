#pragma once

#include "event.hpp"
#include "game_layer.hpp"
#include "layer.hpp"
#include "window.hpp"

#include <memory>

namespace dc
{

class EditorLayer : public Layer
{
public:
  void init() override;
  void shutdown() override;
  void update(float delta_time) override;
  void render() override;

  bool on_event(const Event &event) override;

private:
  std::unique_ptr<GameLayer> game_layer_{};

  void setup_game();
  void set_capture_mouse(bool value);
  bool on_key_event(const KeyEvent &event);
};

} // namespace dc
