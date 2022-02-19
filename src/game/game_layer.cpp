#include "game_layer.hpp"
#include "camera_component.hpp"
#include "camera_system.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "model_component.hpp"
#include "render_system.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "texture_cache.hpp"
#include "transform_component.hpp"

#include <memory>

void GameLayer::init()
{
  renderer_ = std::make_unique<Renderer>();

  TextureCache texture_cache;
  texture_cache.set_import_path(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF");

  // load_scene
  // scene_                        = Scene::create();
  scene_ = Scene::load_from_file(
      "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
      texture_cache);
  const auto scene_loaded_event = std::make_shared<SceneLoadedEvent>(scene_);
  Engine::instance()->event_manager()->publish(scene_loaded_event);

  // add systems, order matters
  scene_->create_system<CameraSystem>(scene_);
  scene_->create_system<RenderSystem>(scene_);

  // create dummy scene
  // auto sponza_model = std::make_shared<Model>();
  // sponza_model->load_from_file(
  //     "external/deps/src/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf",
  //     texture_cache);

  // auto  sponza_entity    = scene_->create_entity("Sponza");
  // auto &model_component  = sponza_entity.add_component<ModelComponent>();
  // model_component.model_ = sponza_model;

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

void GameLayer::shutdown() {}

void GameLayer::update(float delta_time)
{
  if (scene_)
  {
    scene_->update(delta_time);
  }
}

void GameLayer::render()
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

std::shared_ptr<Renderer> GameLayer::renderer() const { return renderer_; }

bool GameLayer::on_event(const Event &event) { return scene_->on_event(event); }
