#include "performance_panel.hpp"
#include "engine.hpp"
#include "imgui_panel.hpp"
#include "profiling.hpp"

namespace dc
{
PerformancePanel::PerformancePanel() : ImGuiPanel("Performance") {}

void PerformancePanel::on_render()
{
  DC_PROFILE_SCOPE("PerformancePanel::on_render()");

  Engine::instance()->performance_profiler()->for_each(
      [](const auto &name, auto time)
      { ImGui::Text("%s: %.3fms\n", name.c_str(), time); });
}

} // namespace dc
