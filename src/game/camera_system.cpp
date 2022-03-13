#include "camera_system.hpp"
#include "profiling.hpp"
#include "camera_component.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "transform_component.hpp"
#include "window.hpp"

namespace dc
{

CameraSystem::CameraSystem(std::weak_ptr<Scene> scene) : scene_{scene} {}

CameraSystem::~CameraSystem() { shutdown(); }

void CameraSystem::shutdown() {}

void CameraSystem::init() {}

void CameraSystem::update(float delta_time)
{
  DC_PROFILE_SCOPE("CameraSystem::update()");

  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto view = scene->all_entities_with<CameraComponent>();
  for (const auto &entity : view)
  {
    auto &camera_component = view.get<CameraComponent>(entity);

    if (!camera_component.active_)
    {
      continue;
    }

    auto      &camera = camera_component.camera_;
    const auto window = Engine::instance()->window();
    // process movement
    camera.clear_movement();
    if (window->key(Key::W) == KeyAction::Press)
    {
      camera.set_move_forward(true);
    }
    if (window->key(Key::S) == KeyAction::Press)
    {
      camera.set_move_backward(true);
    }
    if (window->key(Key::A) == KeyAction::Press)
    {
      camera.set_move_left(true);
    }
    if (window->key(Key::D) == KeyAction::Press)
    {
      camera.set_move_right(true);
    }
    camera.update_movement(delta_time);
  }
}

void CameraSystem::render(SceneRenderInfo & /*scene_render_info*/,
                          ViewRenderInfo &view_render_info)
{
  DC_PROFILE_SCOPE("CameraSystem::render()");

  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto  view = scene->all_entities_with<CameraComponent>();
  bool  found{false};
  for (const auto &entity : view)
  {
    const auto &camera_component = view.get<CameraComponent>(entity);

    if (!camera_component.active_)
    {
      continue;
    }

    auto &camera = camera_component.camera_;
    view_render_info.set_aspect_ratio(camera.aspect_ratio());
    view_render_info.set_fov(camera.zoom());
    view_render_info.set_near_plane(camera.near_plane());
    view_render_info.set_far_plane(camera.far_plane());
    view_render_info.set_view_matrix(camera.view_matrix());
    view_render_info.set_projection_matrix(camera.projection_matrix());

    if (found)
    {
      DC_LOG_WARN("More than one camera is active");
      break;
    }
    found = true;
  }
}

bool CameraSystem::on_mouse_movement(const MouseMovementEvent &event)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return false;
  }

  auto view = scene->all_entities_with<CameraComponent>();
  for (const auto &entity : view)
  {
    auto &camera_component = view.get<CameraComponent>(entity);

    if (!camera_component.active_)
    {
      continue;
    }

    auto &camera = camera_component.camera_;
    camera.update_rotation(event.offset_x_, event.offset_y_);
  }

  return false;
}

bool CameraSystem::on_window_resize(const WindowResizeEvent &event)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return false;
  }

  auto view = scene->all_entities_with<CameraComponent>();
  for (const auto &entity : view)
  {
    auto &camera_component = view.get<CameraComponent>(entity);

    if (!camera_component.active_)
    {
      continue;
    }

    auto &camera = camera_component.camera_;
    camera.set_aspect_ratio(static_cast<float>(event.width_) / event.height_);
  }

  return false;
}

bool CameraSystem::on_event(const Event &event)
{
  const auto event_id = event.id();

  if (event_id == MouseMovementEvent::id)
  {
    return on_mouse_movement(dynamic_cast<const MouseMovementEvent &>(event));
  }
  else if (event_id == WindowResizeEvent::id)
  {
    return on_window_resize(dynamic_cast<const WindowResizeEvent &>(event));
  }

  return false;
}

} // namespace dc
