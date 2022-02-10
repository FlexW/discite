#include "camera_system.hpp"
#include "camera_component.hpp"
#include "engine/camera.hpp"
#include "engine/engine.hpp"
#include "engine/log.hpp"
#include "engine/window.hpp"
#include "transform_component.hpp"

CameraSystem::CameraSystem(std::weak_ptr<Scene> scene) : scene_{scene} {}

CameraSystem::~CameraSystem() { shutdown(); }

void CameraSystem::shutdown()
{
  const auto event_manager = Engine::instance()->event_manager();

  event_manager->unsubscribe(
      fastdelegate::MakeDelegate(this, &CameraSystem::on_window_resize),
      WindowResizeEvent::id);
  event_manager->unsubscribe(
      fastdelegate::MakeDelegate(this, &CameraSystem::on_mouse_movement),
      MouseMovementEvent::id);
}

void CameraSystem::init()
{
  const auto event_manager = Engine::instance()->event_manager();

  event_manager->subscribe(
      fastdelegate::MakeDelegate(this, &CameraSystem::on_window_resize),
      WindowResizeEvent::id);
  event_manager->subscribe(
      fastdelegate::MakeDelegate(this, &CameraSystem::on_mouse_movement),
      MouseMovementEvent::id);
}

void CameraSystem::update(float delta_time)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &registry = scene->registry();
  auto  view     = registry.view<CameraComponent>();
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
    if (window->key(Key::W) == KeyAction::Press)
    {
      camera.process_movement(CameraMovement::Forward, delta_time);
    }
    if (window->key(Key::S) == KeyAction::Press)
    {
      camera.process_movement(CameraMovement::Backward, delta_time);
    }
    if (window->key(Key::A) == KeyAction::Press)
    {
      camera.process_movement(CameraMovement::Left, delta_time);
    }
    if (window->key(Key::D) == KeyAction::Press)
    {
      camera.process_movement(CameraMovement::Right, delta_time);
    }
  }
}

void CameraSystem::render(SceneRenderInfo & /*scene_render_info*/,
                          ViewRenderInfo &view_render_info)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &registry = scene->registry();
  auto  view     = registry.view<CameraComponent>();
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
      LOG_WARN() << "More than one camera is active";
      break;
    }
    found = true;
  }
}

void CameraSystem::on_mouse_movement(const Event &event)
{
  const auto &mouse_movement_event =
      dynamic_cast<const MouseMovementEvent &>(event);

  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &registry = scene->registry();
  auto  view     = registry.view<CameraComponent>();
  for (const auto &entity : view)
  {
    auto &camera_component = view.get<CameraComponent>(entity);

    if (!camera_component.active_)
    {
      continue;
    }

    auto &camera = camera_component.camera_;
    camera.process_rotation(mouse_movement_event.offset_x_,
                            mouse_movement_event.offset_y_);
  }
}

void CameraSystem::on_window_resize(const Event &event)
{
  const auto &window_resize_event =
      dynamic_cast<const WindowResizeEvent &>(event);

  const auto scene = scene_.lock();
  if (!scene)
  {
    return;
  }

  auto &registry = scene->registry();
  auto  view     = registry.view<CameraComponent>();
  for (const auto &entity : view)
  {
    auto &camera_component = view.get<CameraComponent>(entity);

    if (!camera_component.active_)
    {
      continue;
    }

    auto &camera = camera_component.camera_;
    camera.set_aspect_ratio(static_cast<float>(window_resize_event.width_) /
                            window_resize_event.height_);
  }
}
