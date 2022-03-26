#pragma once

#include "camera.hpp"
#include "entity.hpp"
#include "event.hpp"
#include "gl_framebuffer.hpp"
#include "imgui_panel.hpp"
#include "scene.hpp"
#include "scene_events.hpp"
#include "scene_panel.hpp"
#include "scene_renderer.hpp"
#include "window.hpp"

#include <ImGuizmo.h>

#include <memory>

namespace dc
{

class PlaySceneEvent : public Event
{
public:
  static EventId id;

  bool play_{true};

  explicit PlaySceneEvent(bool play);
};

class ViewportPanel : public ImGuiPanel
{
public:
  ViewportPanel();

  void update(float delta_time) override;
  bool on_event(const Event &event) override;

private:
  Camera editor_camera_;
  bool   is_move_editor_camara_{false};

  bool is_playing_{false};

  int scene_width_{0};
  int scene_height_{0};

  Entity              selected_entity_;
  bool                is_show_gizmo_{false};
  ImGuizmo::OPERATION guizmo_operation_{ImGuizmo::OPERATION::TRANSLATE};

  std::weak_ptr<Scene>           scene_{};
  std::shared_ptr<GlFramebuffer> scene_framebuffer_{};

  void on_render() override;
  ImGuiWindowFlags on_before_render() override;
  void on_after_render() override;

  glm::ivec2 calc_viewport_size() const;
  bool       is_window_size_changed(int width, int height);
  void       account_for_window_size_changes(int width, int height);
  void       recreate_projection_matrix();
  void       recreate_scene_framebuffer();

  void start_move_editor_camera();
  void stop_move_editor_camera();
  void set_capture_mouse(bool value);

  bool on_key(const KeyEvent &event);
  bool on_mouse_button(const MouseButtonEvent &event);
  bool on_mouse_movement(const MouseMovementEvent &event);
  bool on_scene_loaded(const SceneLoadedEvent &event);
  bool on_scene_unloaded(const SceneUnloadedEvent &event);
  bool on_entity_selected(const EntitySelectedEvent &event);

  void move_editor_camera(float delta_time);
  void rotate_editor_camera(double offset_x, double offset_y);
};

} // namespace dc
