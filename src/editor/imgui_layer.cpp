#include "imgui_layer.hpp"
#include "engine.hpp"
#include "imgui.h"
#include "imgui.hpp"
#include "util.hpp"
#include "window.hpp"

#include <ImGuizmo.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void ImGuiLayer::init()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  auto &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();

  auto &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding              = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  // TODO: This is a hidden dependecy on GLFW and OpenGL3
  ImGui_ImplGlfw_InitForOpenGL(Engine::instance()->window()->handle(), false);
  ImGui_ImplOpenGL3_Init("#version 460 core");
}

void ImGuiLayer::shutdown()
{
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiLayer::update(float delta_time)
{
  for (const auto &panel : panels_)
  {
    panel->update(delta_time);
  }
}

void ImGuiLayer::render()
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGuizmo::BeginFrame();

  render_panels();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  auto &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    auto backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

bool ImGuiLayer::on_event(const Event &event)
{
  for (auto &panel : panels_)
  {
    if (panel->on_event(event))
    {
      return true;
    }
  }

  const auto event_id = event.id();

  if (event_id == KeyEvent::id)
  {
    return on_key_event(dynamic_cast<const KeyEvent &>(event));
  }
  else if (event_id == WindowFocusEvent::id)
  {
    return on_windows_focus_event(
        dynamic_cast<const WindowFocusEvent &>(event));
  }
  else if (event_id == MouseMovementEvent::id)
  {
    return on_mouse_movement_event(
        dynamic_cast<const MouseMovementEvent &>(event));
  }
  else if (event_id == CursorEnterEvent::id)
  {
    return on_cursor_enter_event(dynamic_cast<const CursorEnterEvent &>(event));
  }
  else if (event_id == MouseButtonEvent::id)
  {
    return on_mouse_button_event(dynamic_cast<const MouseButtonEvent &>(event));
  }
  else if (event_id == ScrollEvent::id)
  {
    return on_scroll_event(dynamic_cast<const ScrollEvent &>(event));
  }
  else if (event_id == CharEvent::id)
  {
    return on_char_event(dynamic_cast<const CharEvent &>(event));
  }
  else if (event_id == MonitorEvent::id)
  {
    return on_monitor_event(dynamic_cast<const MonitorEvent &>(event));
  }

  return false;
}

bool ImGuiLayer::on_key_event(const KeyEvent &event)
{
  ImGui_ImplGlfw_KeyCallback(Engine::instance()->window()->handle(),
                             to_glfw(event.key_),
                             event.scancode_,
                             to_glfw(event.key_action_),
                             0);
  return true;
}

bool ImGuiLayer::on_windows_focus_event(const WindowFocusEvent &event)
{
  ImGui_ImplGlfw_WindowFocusCallback(Engine::instance()->window()->handle(),
                                     event.focused_);
  return true;
}

bool ImGuiLayer::on_scroll_event(const ScrollEvent &event)
{
  ImGui_ImplGlfw_ScrollCallback(Engine::instance()->window()->handle(),
                                event.x_offset_,
                                event.y_offset_);
  return true;
}

bool ImGuiLayer::on_cursor_enter_event(const CursorEnterEvent &event)
{
  ImGui_ImplGlfw_CursorEnterCallback(Engine::instance()->window()->handle(),
                                     event.entered_);
  return true;
}

bool ImGuiLayer::on_mouse_button_event(const MouseButtonEvent &event)
{
  ImGui_ImplGlfw_MouseButtonCallback(Engine::instance()->window()->handle(),
                                     to_glfw(event.mouse_button_),
                                     to_glfw(event.mouse_button_action_),
                                     0);
  return true;
}

bool ImGuiLayer::on_char_event(const CharEvent &event)
{
  ImGui_ImplGlfw_CharCallback(Engine::instance()->window()->handle(),
                              event.character_);
  return true;
}

bool ImGuiLayer::on_monitor_event(const MonitorEvent &event)
{
  ImGui_ImplGlfw_MonitorCallback(event.monitor_->handle(), event.event_);
  return true;
}

bool ImGuiLayer::on_mouse_movement_event(const MouseMovementEvent &event)
{
  ImGui_ImplGlfw_CursorPosCallback(Engine::instance()->window()->handle(),
                                   event.x_,
                                   event.y_);
  return true;
}

void ImGuiLayer::add_panel(std::shared_ptr<ImGuiPanel> panel)
{
  panels_.push_back(panel);
}

void ImGuiLayer::render_panels()
{
  for (const auto &panel : panels_)
  {
    panel->render();
  }
}
