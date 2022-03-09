#include "viewport_panel.hpp"
#include "ImGuizmo.h"
#include "engine.hpp"
#include "gl_framebuffer.hpp"
#include "gl_texture.hpp"
#include "glm/ext/quaternion_common.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui.h"
#include "imgui_panel.hpp"
#include "relationship_component.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "scene_panel.hpp"
#include "window.hpp"

#include <memory>

namespace dc
{

ViewportPanel::ViewportPanel() : ImGuiPanel{"Viewport"} {}

void ViewportPanel::update(float delta_time) { move_editor_camera(delta_time); }

void ViewportPanel::on_render()
{
  const auto viewport_size = calc_viewport_size();
  account_for_window_size_changes(viewport_size.x, viewport_size.y);

  const auto scene    = scene_.lock();
  const auto renderer = renderer_.lock();
  if (scene)
  {
    SceneRenderInfo scene_render_info{};
    ViewRenderInfo  view_render_info{};
    scene->render(scene_render_info, view_render_info);

    // set editor camera information
    view_render_info.set_projection_matrix(editor_camera_.projection_matrix());
    view_render_info.set_near_plane(editor_camera_.near_plane());
    view_render_info.set_far_plane(editor_camera_.far_plane());
    view_render_info.set_aspect_ratio(editor_camera_.aspect_ratio());
    view_render_info.set_fov(editor_camera_.zoom());
    view_render_info.set_view_matrix(editor_camera_.view_matrix());
    view_render_info.set_viewport_info({0, 0, scene_width_, scene_height_});

    renderer->render(scene_render_info,
                     view_render_info,
                     scene_framebuffer_.get());
  }
  else
  {
    // no scene set, black out framebuffer to not have garbage on the screen
    scene_framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    scene_framebuffer_->unbind();
  }

  const auto scene_texture = std::get<std::shared_ptr<GlTexture>>(
      scene_framebuffer_->color_attachment(0));
  ImGui::Image(reinterpret_cast<void *>(scene_texture->id()),
               ImVec2{static_cast<float>(scene_width_),
                      static_cast<float>(scene_height_)},
               ImVec2{0.0f, 1.0f},
               ImVec2{1.0f, 0.0f});

  if (is_show_gizmo_ && selected_entity_.valid())
  {
    // draw guizmo
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    const auto [window_positon_x, window_position_y] = ImGui::GetWindowPos();
    ImGuizmo::SetRect(window_positon_x,
                      window_position_y,
                      viewport_size.x,
                      viewport_size.y);

    const auto &projection_matrix       = editor_camera_.projection_matrix();
    const auto &view_matrix             = editor_camera_.view_matrix();
    auto        entity_transform_matrix = selected_entity_.transform_matrix();

    bool is_snap{false};
    if (Engine::instance()->window()->key(Key::LeftShift) == KeyAction::Press)
    {
      is_snap = true;
    }
    const glm::vec3 snap_values{
        guizmo_operation_ == ImGuizmo::OPERATION::ROTATE ? 45.0f : 0.5f};

    ImGuizmo::Manipulate(glm::value_ptr(view_matrix),
                         glm::value_ptr(projection_matrix),
                         guizmo_operation_,
                         ImGuizmo::LOCAL,
                         glm::value_ptr(entity_transform_matrix),
                         nullptr,
                         is_snap ? glm::value_ptr(snap_values) : nullptr);

    if (ImGuizmo::IsUsing())
    {
      auto &relationship_component =
          selected_entity_.component<RelationshipComponent>();
      if (relationship_component.parent_.valid())
      {
        // account for parent transformation
        const auto parent_transform =
            relationship_component.parent_.transform_matrix();
        entity_transform_matrix =
            glm::inverse(parent_transform) * entity_transform_matrix;
      }
      selected_entity_.set_local_transform_matrix(entity_transform_matrix);
    }
  }
}

bool ViewportPanel::on_event(const Event &event)
{

  const auto event_id = event.id();
  if (event_id == SceneLoadedEvent::id)
  {
    return on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }
  else if (event_id == KeyEvent::id)
  {
    return on_key(dynamic_cast<const KeyEvent &>(event));
  }
  else if (event_id == MouseMovementEvent::id)
  {
    return on_mouse_movement(dynamic_cast<const MouseMovementEvent &>(event));
  }
  else if (event_id == MouseButtonEvent::id)
  {
    return on_mouse_button(dynamic_cast<const MouseButtonEvent &>(event));
  }
  else if (event_id == EntitySelectedEvent::id)
  {
    return on_entity_selected(dynamic_cast<const EntitySelectedEvent &>(event));
  }
  else if (event_id == SceneUnloadedEvent::id)
  {
    return on_scene_unloaded(dynamic_cast<const SceneUnloadedEvent &>(event));
  }
  return false;
}

ImGuiWindowFlags ViewportPanel::on_before_render()
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  return ImGuiWindowFlags_None;
}

void ViewportPanel::on_after_render() { ImGui::PopStyleVar(); }

glm::ivec2 ViewportPanel::calc_viewport_size() const
{
  auto viewport_size = ImGui::GetContentRegionAvail();

  if (0 >= viewport_size.x ||
      static_cast<std::uint32_t>(viewport_size.x) >=
          std::numeric_limits<std::uint32_t>::max() ||
      0 >= viewport_size.y ||
      static_cast<std::uint32_t>(viewport_size.y) >=
          std::numeric_limits<std::uint32_t>::max())
  {
    // If window is too small provide a fallback
    viewport_size.x = 100.0;
    viewport_size.y = 100.0;
  }

  return {viewport_size.x, viewport_size.y};
}
bool ViewportPanel::is_window_size_changed(int width, int height)
{
  return (width != scene_width_ || height != scene_height_);
}

void ViewportPanel::account_for_window_size_changes(int width, int height)
{
  if (!is_window_size_changed(width, height))
  {
    return;
  }

  scene_width_  = width;
  scene_height_ = height;

  recreate_scene_framebuffer();
  recreate_projection_matrix();
}

void ViewportPanel::recreate_projection_matrix()
{
  editor_camera_.set_aspect_ratio(static_cast<float>(scene_width_) /
                                  scene_height_);
}

void ViewportPanel::recreate_scene_framebuffer()
{
  FramebufferAttachmentCreateConfig color_config{};
  color_config.type_            = AttachmentType::Texture;
  color_config.format_          = GL_RGBA;
  color_config.internal_format_ = GL_RGBA16F;
  color_config.width_           = scene_width_;
  color_config.height_          = scene_height_;

  FramebufferAttachmentCreateConfig depth_config{};
  depth_config.type_            = AttachmentType::Renderbuffer;
  depth_config.format_          = GL_DEPTH_COMPONENT;
  depth_config.internal_format_ = GL_DEPTH_COMPONENT32F;
  depth_config.width_           = scene_width_;
  depth_config.height_          = scene_height_;

  FramebufferConfig config{};
  config.color_attachments_.push_back(color_config);
  config.depth_attachment_ = depth_config;

  scene_framebuffer_ = std::make_shared<GlFramebuffer>();
  scene_framebuffer_->attach(config);
}

bool ViewportPanel::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
  return false;
}

bool ViewportPanel::on_key(const KeyEvent &event)
{
  if (is_move_editor_camara_)
  {
    // make sure key events do not propagate to other layers (imgui)
    return true;
  }

  const auto could_show_gizmo = is_hovered() && selected_entity_.valid();
  if (is_hovered() && event.key_ == Key::Q)
  {
    is_show_gizmo_ = false;
    return true;
  }
  else if (could_show_gizmo && event.key_ == Key::G)
  {
    is_show_gizmo_    = true;
    guizmo_operation_ = ImGuizmo::OPERATION::TRANSLATE;
    return true;
  }
  else if (could_show_gizmo && event.key_ == Key::S)
  {
    is_show_gizmo_    = true;
    guizmo_operation_ = ImGuizmo::OPERATION::SCALE;
    return true;
  }
  else if (could_show_gizmo && event.key_ == Key::R)
  {
    is_show_gizmo_    = true;
    guizmo_operation_ = ImGuizmo::OPERATION::ROTATE;
    return true;
  }

  return false;
}

bool ViewportPanel::on_mouse_movement(const MouseMovementEvent &event)
{
  rotate_editor_camera(event.offset_x_, event.offset_y_);

  if (is_move_editor_camara_)
  {
    // make sure mouse movements events do not propagate to other layers (imgui)
    return true;
  }

  return false;
}

bool ViewportPanel::on_mouse_button(const MouseButtonEvent &event)
{
  if (event.mouse_button_ == MouseButton::Right &&
      event.mouse_button_action_ == MouseButtonAction::Press && is_hovered())
  {
    start_move_editor_camera();
  }
  else if (event.mouse_button_ == MouseButton::Right &&
           event.mouse_button_action_ == MouseButtonAction::Release)
  {
    stop_move_editor_camera();
  }

  if (is_move_editor_camara_)
  {
    // make sure mouse button events do not propagate to other layers (imgui)
    return true;
  }
  return false;
}

bool ViewportPanel::on_scene_unloaded(const SceneUnloadedEvent & /*event*/)
{
  is_show_gizmo_   = false;
  selected_entity_ = {};
  return false;
}

bool ViewportPanel::on_entity_selected(const EntitySelectedEvent &event)
{
  selected_entity_ = event.entity_;
  return false;
}

void ViewportPanel::start_move_editor_camera()
{
  is_move_editor_camara_ = true;
  set_capture_mouse(true);
}

void ViewportPanel::stop_move_editor_camera()
{
  if (!is_move_editor_camara_)
  {
    return;
  }

  is_move_editor_camara_ = false;
  set_capture_mouse(false);
}

void ViewportPanel::set_capture_mouse(bool value)
{
  const auto window = Engine::instance()->window();
  window->set_capture_mouse(value);
}

void ViewportPanel::move_editor_camera(float delta_time)
{
  if (!is_move_editor_camara_)
  {
    return;
  }

  const auto window = Engine::instance()->window();
  if (window->key(Key::W) == KeyAction::Press)
  {
    editor_camera_.process_movement(CameraMovement::Forward, delta_time);
  }
  if (window->key(Key::S) == KeyAction::Press)
  {
    editor_camera_.process_movement(CameraMovement::Backward, delta_time);
  }
  if (window->key(Key::A) == KeyAction::Press)
  {
    editor_camera_.process_movement(CameraMovement::Left, delta_time);
  }
  if (window->key(Key::D) == KeyAction::Press)
  {
    editor_camera_.process_movement(CameraMovement::Right, delta_time);
  }
}

void ViewportPanel::rotate_editor_camera(double offset_x, double offset_y)
{
  if (!is_move_editor_camara_)
  {
    return;
  }

  editor_camera_.process_rotation(offset_x, offset_y);
}

void ViewportPanel::set_renderer(std::shared_ptr<Renderer> renderer)
{
  renderer_ = renderer;
}

} // namespace dc
