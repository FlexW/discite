#include "imgui_panel.hpp"
#include "imgui.h"
#include "imgui.hpp"

ImGuiPanel::ImGuiPanel(const std::string &name) : name_{name} {}

void ImGuiPanel::update(float /*delta_time*/) {}

void ImGuiPanel::render()
{
  const auto window_flags = on_before_render();
  ImGui::Begin(name_.c_str(), nullptr, window_flags);
  focused_ = ImGui::IsWindowFocused();
  hovered_ = ImGui::IsWindowHovered();
  on_render();
  ImGui::End();
  on_after_render();
}

bool ImGuiPanel::is_focused() const { return focused_; }

bool ImGuiPanel::is_hovered() const { return hovered_; }

ImGuiWindowFlags ImGuiPanel::on_before_render()
{
  return ImGuiWindowFlags_None;
}

void ImGuiPanel::on_after_render() {}
