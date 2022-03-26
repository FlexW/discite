#include "render_system.hpp"
#include "directional_light.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "frame_data.hpp"
#include "game_layer.hpp"
#include "log.hpp"
#include "model_component.hpp"
#include "point_light.hpp"
#include "point_light_component.hpp"
#include "profiling.hpp"
#include "sky_component.hpp"
#include "transform_component.hpp"

#include <memory>

namespace dc
{

void RenderSystem::init()
{
  const auto game_layer = Engine::instance()->layer_stack()->layer<GameLayer>();
  if (game_layer)
  {
    scene_ = game_layer->scene()->get();
  }
}

void RenderSystem::update(float /*delta_time*/) {}

void RenderSystem::render(SceneRenderInfo &scene_render_info,
                          ViewRenderInfo & /*view_render_info*/)
{
  DC_PROFILE_SCOPE("RenderSystem::render()");

  const auto scene = scene_.lock();
  if (!scene)
  {
    DC_LOG_WARN("Scene is not valid. Will not render meshes");
  }

  // add meshes
  {
    DC_PROFILE_SCOPE("RenderSystem::render() - Process meshes");

    auto view = scene->all_entities_with<TransformComponent, ModelComponent>();
    for (const auto entity : view)
    {
      const auto &transform_component = view.get<TransformComponent>(entity);
      const auto &model_component     = view.get<ModelComponent>(entity);

      const auto &model = model_component.model_;
      if (!model || !model->is_ready())
      {
        continue;
      }

      for (const auto &mesh : model->get()->meshes())
      {
        MeshInfo mesh_info{};
        mesh_info.mesh_         = mesh;
        mesh_info.model_matrix_ = transform_component.transform_matrix();
        scene_render_info.add_mesh(mesh_info);
      }
    }
  }

  // add point lights
  {
    DC_PROFILE_SCOPE("RenderSystem::render() - Process point lights");

    auto view =
        scene->all_entities_with<TransformComponent, PointLightComponent>();
    for (const auto entity : view)
    {
      const auto &transform_component   = view.get<TransformComponent>(entity);
      const auto &point_light_component = view.get<PointLightComponent>(entity);

      PointLight point_light{};
      point_light.set_position(transform_component.position());

      point_light.set_color(point_light_component.color_);
      point_light.set_multiplier(point_light_component.multiplier_);

      point_light.set_radius(point_light_component.radius_);
      point_light.set_falloff(point_light_component.falloff_);

      auto shadow_tex = point_light_component.shadow_tex();
      point_light.set_shadow_tex(shadow_tex);
      point_light.set_cast_shadow(shadow_tex != nullptr);

      scene_render_info.add_point_light(point_light);
    }
  }

  // add sky
  {
    DC_PROFILE_SCOPE("RenderSystem::render() - Process sky");

    auto view = scene->all_entities_with<SkyComponent>();
    bool found{false};
    for (const auto &entity : view)
    {
      const auto &sky_component = view.get<SkyComponent>(entity);

      if (sky_component.environment_ && sky_component.environment_->is_ready())
      {
        Entity e{entity, scene};
        scene_render_info.set_env_map(*sky_component.environment_->get());
      }

      if (found)
      {
        DC_LOG_WARN("More than one sky is not supported");
        break;
      }
      found = true;
    }
  }

  // add directional light
  {
    DC_PROFILE_SCOPE("RenderSystem::render() - Process directional light");

    auto view = scene->all_entities_with<TransformComponent,
                                         DirectionalLightComponent>();
    bool found{false};
    for (const auto &entity : view)
    {
      const auto &directional_light_component =
          view.get<DirectionalLightComponent>(entity);

      Entity     e{entity, scene};
      const auto rotation_matrix =
          glm::toMat4(e.component<TransformComponent>().rotation_quat());
      const glm::vec3 rotation{rotation_matrix *
                               glm::vec4{0.0f, -1.0f, 0.0f, 0.0f}};

      DirectionalLight directional_light{};
      directional_light.set_direction(rotation);
      directional_light.set_color(directional_light_component.color_);
      directional_light.set_multiplier(directional_light_component.multiplier_);
      directional_light.set_cast_shadow(
          directional_light_component.cast_shadow_);

      scene_render_info.set_directional_light(directional_light);

      if (found)
      {
        DC_LOG_WARN("More than one directional light is not supported");
        break;
      }
      found = true;
    }
  }
}

bool RenderSystem::on_event(const Event &event)
{
  const auto event_id = event.id();
  if (event_id == SceneLoadedEvent::id)
  {
    on_scene_loaded(dynamic_cast<const SceneLoadedEvent &>(event));
  }

  return false;
}

void RenderSystem::on_scene_loaded(const SceneLoadedEvent &event)
{
  scene_ = event.scene_;
}

} // namespace dc
