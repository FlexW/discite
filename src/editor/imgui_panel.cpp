#include "imgui_panel.hpp"
#include "imgui.h"
#include "imgui.hpp"

ImGuiPanel::ImGuiPanel(const std::string &name) : name_{name} {}

void ImGuiPanel::update(float /*delta_time*/) {}

void ImGuiPanel::render()
{
  on_before_render();
  ImGui::Begin(name_.c_str());
  focused_ = ImGui::IsWindowFocused();
  on_render();
  ImGui::End();
  on_after_render();
}

bool ImGuiPanel::is_focused() const { return focused_; }

void ImGuiPanel::on_before_render() {}

void ImGuiPanel::on_after_render() {}
