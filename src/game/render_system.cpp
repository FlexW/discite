#include "render_system.hpp"
#include "directional_light.hpp"
#include "directional_light_component.hpp"
#include "glm/gtx/quaternion.hpp"
#include "log.hpp"
#include "model_component.hpp"
#include "point_light.hpp"
#include "point_light_component.hpp"
#include "renderer.hpp"
#include "transform_component.hpp"

#include <memory>

RenderSystem::RenderSystem(std::weak_ptr<Scene> scene) : scene_{scene} {}

void RenderSystem::init() {}

void RenderSystem::update(float /*delta_time*/) {}

void RenderSystem::render(SceneRenderInfo &scene_render_info,
                          ViewRenderInfo & /*view_render_info*/)
{
  const auto scene = scene_.lock();
  if (!scene)
  {
    LOG_WARN() << "Scene is not valid. Will not render meshes";
  }

  auto &registry = scene->registry();

  // add meshes
  {
    auto view = registry.view<TransformComponent, ModelComponent>();
    for (const auto entity : view)
    {
      const auto &transform_component = view.get<TransformComponent>(entity);
      const auto &model_component     = view.get<ModelComponent>(entity);

      const auto &model = model_component.model_;
      if (!model)
      {
        continue;
      }

      for (const auto &mesh : model->meshes())
      {
        MeshInfo mesh_info{};
        mesh_info.mesh_             = mesh;
        mesh_info.position_         = transform_component.position();
        mesh_info.rotation_         = transform_component.rotation();
        mesh_info.scale_            = transform_component.scale();
        mesh_info.parent_transform_matrix_ =
            transform_component.parent_transform_matrix();
        mesh_info.model_matrix_ = transform_component.transform_matrix();
        scene_render_info.add_mesh(mesh_info);
      }
    }
  }

  // add point lights
  {
    auto view = registry.view<TransformComponent, PointLightComponent>();
    for (const auto entity : view)
    {
      const auto &transform_component   = view.get<TransformComponent>(entity);
      const auto &point_light_component = view.get<PointLightComponent>(entity);

      PointLight point_light{};
      point_light.set_position(transform_component.position());

      point_light.set_ambient_color(point_light_component.ambient_color_);
      point_light.set_diffuse_color(point_light_component.diffuse_color_);
      point_light.set_specular_color(point_light_component.specular_color_);

      point_light.set_constant(point_light_component.constant_);
      point_light.set_linear(point_light_component.linear_);
      point_light.set_quadratic(point_light_component.quadratic_);

      scene_render_info.add_point_light(point_light);
    }
  }

  // add directional light
  {
    auto view = registry.view<TransformComponent, DirectionalLightComponent>();
    bool found{false};
    for (const auto &entity : view)
    {
      // const auto &transform_component = view.get<TransformComponent>(entity);
      const auto &directional_light_component =
          view.get<DirectionalLightComponent>(entity);

      DirectionalLight directional_light{};
      // const auto       rotation_matrix =
      //     glm::toMat4(transform_component.rotation_quat());
      // const auto direction =
      //     rotation_matrix * glm::vec4{glm::vec3{0.0f, 0.0f, 1.0f}, 0.0f};
      const glm::vec3 direction{0.1f, -1.0f, -0.1f};
      directional_light.set_direction(direction);

      directional_light.set_ambient_color(
          directional_light_component.ambient_color_);
      directional_light.set_diffuse_color(
          directional_light_component.diffuse_color_);
      directional_light.set_specular_color(
          directional_light_component.specular_color_);

      scene_render_info.set_directional_light(directional_light);

      if (found)
      {
        LOG_WARN() << "More then one directional light is not supported";
        break;
      }
      found = true;
    }
  }
}
