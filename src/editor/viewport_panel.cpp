#include "viewport_panel.hpp"
#include "engine.hpp"
#include "event.hpp"
#include "game_layer.hpp"
#include "gl_framebuffer.hpp"
#include "gl_texture.hpp"
#include "imgui.h"
#include "imgui_panel.hpp"
#include "profiling.hpp"
#include "relationship_component.hpp"
#include "scene.hpp"
#include "scene_panel.hpp"
#include "scene_renderer.hpp"
#include "window.hpp"

#include <ImGuizmo.h>

#include <memory>

namespace dc
{

EventId PlaySceneEvent::id{0xb20f905e};

PlaySceneEvent::PlaySceneEvent(bool play) : Event{id}, play_{play} {}

ViewportPanel::ViewportPanel() : ImGuiPanel{"Viewport"}
{
  editor_camera_.set_enable_acceleration(true);
}

void ViewportPanel::update(float delta_time) { move_editor_camera(delta_time); }

void ViewportPanel::on_render()
{
  DC_PROFILE_SCOPE("ViewportPanel::on_render()");

  const std::string play_button_text{is_playing_ ? "Stop" : "Play"};
  if (ImGui::Button(play_button_text.c_str()))
  {
    is_playing_ = !is_playing_;

    const auto play_scene_event = std::make_shared<PlaySceneEvent>(is_playing_);
    Engine::instance()->event_manager()->queue_event(play_scene_event);
  }

  const auto viewport_size = calc_viewport_size();
  account_for_window_size_changes(viewport_size.x, viewport_size.y);

  const auto game_layer = Engine::instance()->layer_stack()->layer<GameLayer>();
  const auto renderer   = game_layer->renderer();
  if (game_layer && renderer)
  {
    DC_PROFILE_SCOPE("ViewportPanel::on_render() - render scene");

    SceneRenderInfo scene_render_info{};
    ViewRenderInfo  view_render_info{};
    game_layer->systems_context()->render(scene_render_info, view_render_info);

    // set editor camera information
    if (is_playing_)
    {
      const auto aspect_ratio =
          static_cast<float>(viewport_size.x) / viewport_size.y;
      view_render_info.set_viewport_info(
          {0, 0, viewport_size.x, viewport_size.y});
      view_render_info.set_aspect_ratio(aspect_ratio);

      const auto projection =
          glm::perspective(glm::radians(view_render_info.fov()),
                           aspect_ratio,
                           view_render_info.near_plane(),
                           view_render_info.far_plane());
      view_render_info.set_projection_matrix(projection);
    }
    else
    {
      view_render_info.set_projection_matrix(
          editor_camera_.projection_matrix());
      view_render_info.set_near_plane(editor_camera_.near_plane());
      view_render_info.set_far_plane(editor_camera_.far_plane());
      view_render_info.set_aspect_ratio(editor_camera_.aspect_ratio());
      view_render_info.set_fov(editor_camera_.zoom());
      view_render_info.set_view_matrix(editor_camera_.view_matrix());
      view_render_info.set_view_position(editor_camera_.position());
      view_render_info.set_viewport_info({0, 0, scene_width_, scene_height_});
    }

    view_render_info.set_framebuffer(scene_framebuffer_.get());
    renderer->render(scene_render_info, view_render_info);
  }
  else
  {
    // no scene set, black out framebuffer to not have garbage on the screen
    scene_framebuffer_->bind();
    constexpr std::array<float, 4> clear_color{0.0f, 0.0f, 0.0f, 1.0f};
    glClearNamedFramebufferfv(scene_framebuffer_->id(),
                              GL_COLOR,
                              0,
                              clear_color.data());
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
  editor_camera_.clear_movement();
  if (window->key(Key::W) == KeyAction::Press)
  {
    editor_camera_.set_move_forward(true);
  }
  if (window->key(Key::S) == KeyAction::Press)
  {
    editor_camera_.set_move_backward(true);
  }
  if (window->key(Key::A) == KeyAction::Press)
  {
    editor_camera_.set_move_left(true);
  }
  if (window->key(Key::D) == KeyAction::Press)
  {
    editor_camera_.set_move_right(true);
  }
  editor_camera_.update_movement(delta_time);
}

void ViewportPanel::rotate_editor_camera(double offset_x, double offset_y)
{
  if (!is_move_editor_camara_)
  {
    return;
  }

  editor_camera_.update_rotation(offset_x, offset_y);
}

} // namespace dc
