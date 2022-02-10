#include "discite.hpp"
#include "camera_component.hpp"
#include "camera_system.hpp"
#include "directional_light_component.hpp"
#include "engine/engine.hpp"
#include "engine/texture_cache.hpp"
#include "entity.hpp"
#include "model_component.hpp"
#include "render_system.hpp"
#include "renderer.hpp"
#include "transform_component.hpp"

#include <memory>

void Discite::init()
{
  renderer_ = std::make_unique<Renderer>();
  scene_    = std::make_shared<Scene>();

  // add systems, order matters
  scene_->create_system<CameraSystem>(scene_);
  scene_->create_system<RenderSystem>(scene_);

  // create dummy scene
  TextureCache texture_cache;
  texture_cache.set_import_path(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF");
  auto sponza_model = std::make_shared<Model>();
  sponza_model->load_from_file(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
      texture_cache);

  auto  sponza_entity    = scene_->create_entity("Sponza");
  auto &model_component  = sponza_entity.add_component<ModelComponent>();
  model_component.model_ = sponza_model;

  auto sun_entity = scene_->create_entity("Sun");
  sun_entity.add_component<DirectionalLightComponent>();
  auto &sun_transform_component = sun_entity.component<TransformComponent>();
  sun_transform_component.set_rotation(
      glm::vec3{glm::radians(-90.0f), 0.0f, 0.0f});

  const auto window        = Engine::instance()->window();
  auto camera_entity = scene_->create_entity("Camera");
  camera_entity.add_component<CameraComponent>(
      0.1f,
      600.0f,
      static_cast<float>(window->width()) / window->height());

  scene_->init();
}

void Discite::update(float delta_time)
{
  if (scene_)
  {
    scene_->update(delta_time);
  }
}

void Discite::render()
{
  SceneRenderInfo scene_render_info{};
  ViewRenderInfo  view_render_info{};
  if (scene_)
  {
    scene_->render(scene_render_info, view_render_info);
  }
  const auto window = Engine::instance()->window();
  view_render_info.set_viewport_info({0, 0, window->width(), window->height()});

  renderer_->submit(scene_render_info, view_render_info);
}

void Discite::render_imgui() { renderer_->render_imgui(); }