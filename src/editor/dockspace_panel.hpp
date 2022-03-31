#pragma once

#include "imgui_panel.hpp"

#include <functional>

namespace dc
{

class DockspacePanel : public ImGuiPanel
{
public:
  std::function<void(const std::string &)> on_new_scene_;

  DockspacePanel();

private:
  std::string new_scene_name_;

  ImGuiWindowFlags on_before_render() override;
  void on_render() override;
};

} // namespace dc
