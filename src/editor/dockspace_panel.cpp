#include "dockspace_panel.hpp"
#include "engine.hpp"
#include "imgui.h"
#include "imgui.hpp"
#include "imgui_panel.hpp"
#include "profiling.hpp"

namespace
{
constexpr ImGuiDockNodeFlags dockspace_flags{ImGuiDockNodeFlags_None};
}

namespace dc
{

DockspacePanel::DockspacePanel() : ImGuiPanel{"Dockspace"} {}

ImGuiWindowFlags DockspacePanel::on_before_render()
{
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  const auto viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |=
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
  {
    window_flags |= ImGuiWindowFlags_NoBackground;
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  return window_flags;
}

void DockspacePanel::on_render()
{
  DC_PROFILE_SCOPE("DockspacePanel::on_render()");

  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);

  auto &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
    const auto dockspace_id = ImGui::GetID("EditorDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  // menubar
  bool is_open_new_scene_popup_{false};

  if (ImGui::BeginMenuBar())
  {
    if (ImGui::BeginMenu("File"))
    {
      if (ImGui::MenuItem("Exit"))
      {
        Engine::instance()->set_close(true);
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Scene"))
    {
      if (ImGui::MenuItem("New"))
      {
        is_open_new_scene_popup_ = true;
        new_scene_name_ = "";
      }
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  // popups
  if (is_open_new_scene_popup_)
  {
    ImGui::OpenPopup("New scene");
  }

  // Always center this window when appearing
  ImVec2 center = ImGui::GetMainViewport()->GetCenter();
  ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
  if (ImGui::BeginPopupModal("New scene",
                             NULL,
                             ImGuiWindowFlags_AlwaysAutoResize))
  {
    imgui_input("Scene name", new_scene_name_);

    if (ImGui::Button("Ok"))
    {
      on_new_scene_(new_scene_name_);
      new_scene_name_ = "";
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
      new_scene_name_ = "";
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

} // namespace dc
