#include "render_system.hpp"
#include "directional_light.hpp"
#include "directional_light_component.hpp"
#include "entity.hpp"
#include "frame_data.hpp"
#include "glm/gtx/quaternion.hpp"
#include "log.hpp"
#include "model_component.hpp"
#include "point_light.hpp"
#include "point_light_component.hpp"
#include "sky_component.hpp"
#include "transform_component.hpp"

#include <memory>

namespace dc
{

RenderSystem::RenderSystem(std::weak_ptr<Scene> scene) : scene_{scene} {}

void RenderSystem::init() {}

void RenderSystem::update(float /*delta_time*/) {}

void RenderSystem::render(SceneRenderInfo &scene_render_info,
                          ViewRenderInfo & /*view_render_info*/)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    DC_LOG_WARN("Scene is not valid. Will not render meshes");
  }

  // add meshes
  {
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
    auto view =
        scene->all_entities_with<TransformComponent, PointLightComponent>();
    for (const auto entity : view)
    {
      const auto &transform_component   = view.get<TransformComponent>(entity);
      const auto &point_light_component = view.get<PointLightComponent>(entity);

      PointLight point_light{};
      point_light.set_position(transform_component.position());

      point_light.set_color(point_light_component.color_);

      point_light.set_constant(point_light_component.constant_);
      point_light.set_linear(point_light_component.linear_);
      point_light.set_quadratic(point_light_component.quadratic_);

      scene_render_info.add_point_light(point_light);
    }
  }

  // add sky
  {
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

} // namespace dc
