#pragma once

#include "event.hpp"
#include "imgui.hpp"
#include "imgui_panel.hpp"
#include "layer.hpp"
#include "window.hpp"

#include <array>
#include <memory>
#include <vector>

namespace dc
{

class ImGuiLayer : public Layer
{
public:
  void init() override;
  void shutdown() override;

  bool update(float delta_time) override;
  bool render() override;

  bool on_event(const Event &event) override;

  void add_panel(std::shared_ptr<ImGuiPanel> panel);

private:
  std::vector<std::shared_ptr<ImGuiPanel>> panels_;

  bool on_key_event(const KeyEvent &event);
  bool on_windows_focus_event(const WindowFocusEvent &event);
  bool on_scroll_event(const ScrollEvent &event);
  bool on_cursor_enter_event(const CursorEnterEvent &event);
  bool on_mouse_button_event(const MouseButtonEvent &event);
  bool on_char_event(const CharEvent &event);
  bool on_monitor_event(const MonitorEvent &event);
  bool on_mouse_movement_event(const MouseMovementEvent &event);

  void render_panels();
};

} // namespace dc
