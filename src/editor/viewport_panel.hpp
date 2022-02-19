#pragma once

#include "camera.hpp"
#include "event.hpp"
#include "gl_framebuffer.hpp"
#include "imgui_panel.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "window.hpp"

#include <memory>

class ViewportPanel : public ImGuiPanel
{
public:
  ViewportPanel();

  void update(float delta_time) override;
  bool on_event(const Event &event) override;

  void set_renderer(std::shared_ptr<Renderer> renderer);

private:
  Camera editor_camera_;
  bool   is_move_editor_camara_{false};

  int scene_width_{0};
  int scene_height_{0};

  std::weak_ptr<Renderer> renderer_{};

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

  bool on_key_event(const KeyEvent &event);
  bool on_mouse_button_event(const MouseButtonEvent &event);
  bool on_mouse_movement_event(const MouseMovementEvent &event);
  bool on_scene_loaded_event(const SceneLoadedEvent &event);

  void move_editor_camera(float delta_time);
  void rotate_editor_camera(double offset_x, double offset_y);
};
