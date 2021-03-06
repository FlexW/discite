#include "camera_system.hpp"
#include "camera.hpp"
#include "camera_component.hpp"
#include "engine.hpp"
#include "game_layer.hpp"
#include "log.hpp"
#include "profiling.hpp"
#include "transform_component.hpp"
#include "window.hpp"

namespace dc
{

void CameraSystem::init()
{
  const auto game_layer = Engine::instance()->layer_stack()->layer<GameLayer>();
  if (game_layer)
  {
    const auto scene = game_layer->scene();
    if (scene)
    {
      scene_ = scene->get();
    }
  }
}

void CameraSystem::update(float /*delta_time*/)
{
  DC_PROFILE_SCOPE("CameraSystem::update()");
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

  auto  view = scene->all_entities_with<CameraComponent, TransformComponent>();
  bool  found{false};
  for (const auto &entity : view)
  {
    const auto &transform_component = view.get<TransformComponent>(entity);
    const auto &camera_component    = view.get<CameraComponent>(entity);

    if (!camera_component.primary_)
    {
      continue;
    }

    const auto &view_matrix =
        glm::inverse(transform_component.transform_matrix());
    view_render_info.set_fov(camera_component.fov_degree_);
    if (camera_component.projection_type_ == ProjectionType::Perspective)
    {
      view_render_info.set_near_plane(camera_component.perspective_near_);
      view_render_info.set_far_plane(camera_component.perspective_far_);
    }
    else
    {
      view_render_info.set_near_plane(camera_component.orthographic_near_);
      view_render_info.set_far_plane(camera_component.orthographic_far_);
    }
    view_render_info.set_view_position(transform_component.position());
    view_render_info.set_view_matrix(view_matrix);
    view_render_info.set_projection_type(camera_component.projection_type_);

    if (found)
    {
      DC_LOG_WARN("More than one camera is active");
      break;
    }
    found = true;
  }
  if (!found)
  {
    DC_LOG_WARN("No camera active");
  }
}

bool CameraSystem::on_event(const Event &event)
{
  const auto event_id = event.id();

  if (event_id == SceneLoadedEvent::id)
  {
    on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }

  return false;
}

void CameraSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
}

} // namespace dc
