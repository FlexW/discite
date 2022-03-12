#pragma once

#include "imgui_panel.hpp"

namespace dc
{

class PerformancePanel : public ImGuiPanel
{
public:
  PerformancePanel();

private:
  void on_render() override;
};

} // namespace dc
