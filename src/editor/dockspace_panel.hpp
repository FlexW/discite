#pragma once

#include "imgui_panel.hpp"

class DockspacePanel : public ImGuiPanel
{
public:
  DockspacePanel();

private:
  ImGuiWindowFlags on_before_render() override;
  void on_render() override;
};
