#pragma once

#include "event.hpp"
#include "imgui.hpp"

class ImGuiPanel
{
public:
  ImGuiPanel(const std::string &name);
  virtual ~ImGuiPanel() = default;

  void render();

  virtual void update(float delta_time);
  virtual bool on_event(const Event & /*event*/) { return false; }

  bool is_focused() const;

private:
  std::string name_;
  bool        focused_{false};

  virtual void on_render() = 0;
  virtual void on_before_render();
  virtual void on_after_render();
};
