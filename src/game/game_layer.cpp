#include "game_layer.hpp"
#include "camera_component.hpp"
#include "camera_system.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "model_component.hpp"
#include "point_light_component.hpp"
#include "render_system.hpp"
#include "renderer.hpp"
#include "scene.hpp"
#include "scene_asset.hpp"
#include "sky_component.hpp"
#include "transform_component.hpp"

#include <memory>

namespace
{

void scene_add_systems(std::shared_ptr<dc::Scene> scene)
{
  // add systems, order matters
  scene->create_system<dc::CameraSystem>(scene);
  scene->create_system<dc::RenderSystem>(scene);
}

} // namespace

namespace dc
{

void GameLayer::register_asset_loaders()
{
  Engine::instance()->asset_cache()->register_asset_loader(".dcscn",
                                                           scene_asset_loader);
}

void GameLayer::init()
{
  renderer_ = std::make_unique<Renderer>();

  is_init_ = true;
  // TODO: Maybe load a main scene in case there is nothing loaded yet
  if (scene_ && scene_->is_ready())
  {
    set_scene(scene_);
  }
}

void GameLayer::shutdown() {}

void GameLayer::update(float delta_time)
{
  if (scene_ && scene_->is_ready())
  {
    scene_->get()->update(delta_time);
  }
}

void GameLayer::render()
{
  SceneRenderInfo scene_render_info{};
  ViewRenderInfo  view_render_info{};
  if (scene_ && scene_->is_ready())
  {
    scene_->get()->render(scene_render_info, view_render_info);
  }
  const auto window = Engine::instance()->window();
  view_render_info.set_viewport_info({0, 0, window->width(), window->height()});

  renderer_->render(scene_render_info, view_render_info);
}

std::shared_ptr<Renderer> GameLayer::renderer() const { return renderer_; }

bool GameLayer::on_event(const Event &event)
{
  if (scene_ && scene_->is_ready())
  {
    return scene_->get()->on_event(event);
  }
  return false;
}

void GameLayer::set_scene(std::shared_ptr<SceneAssetHandle> value)
{
  if (!is_init_)
  {
    scene_ = value;
    return;
  }

  if (scene_)
  {
    const auto event = std::make_shared<SceneUnloadedEvent>(scene_->get());
    Engine::instance()->event_manager()->publish(event);
  }

  scene_ = value;
  scene_add_systems(scene_->get());

  const auto scene_loaded_event =
      std::make_shared<SceneLoadedEvent>(scene_->get());
  Engine::instance()->event_manager()->publish(scene_loaded_event);
}

std::shared_ptr<SceneAssetHandle> GameLayer::scene() const { return scene_; }

} // namespace dc
