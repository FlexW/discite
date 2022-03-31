#include "game_layer.hpp"
#include "animation_system.hpp"
#include "camera_component.hpp"
#include "camera_system.hpp"
#include "directional_light_component.hpp"
#include "engine.hpp"
#include "entity.hpp"
#include "profiling.hpp"
#include "render_system.hpp"
#include "script/script_system.hpp"
#include "sky_component.hpp"
#include "systems_context.hpp"
#include "transform_component.hpp"

#include <memory>

namespace
{

void add_systems(dc::SystemsContext &system_context)
{
  // add systems, order matters
  system_context.add_system<dc::ScriptSystem>();
  system_context.add_system<dc::CameraSystem>();
  system_context.add_system<dc::AnimationSystem>();
  system_context.add_system<dc::RenderSystem>();
}

} // namespace

namespace dc
{

GameLayer::GameLayer() { add_systems(systems_context_); }

void GameLayer::register_asset_loaders()
{
  Engine::instance()->asset_cache()->register_asset_loader(".dcscn",
                                                           scene_asset_loader);
}

void GameLayer::init()
{
  if (is_init_)
  {
    return;
  }

  is_init_ = true;

  renderer_ = std::make_unique<SceneRenderer>();
  systems_context_.init();
  scene_manager_.init();

  if (!scene_manager_.active_scene())
  {
    // TODO: Load main scene from config file
    scene_manager_.load_scene("scenes/sponza.dcscn");
  }
}

void GameLayer::shutdown() { systems_context_.shutdown(); }

bool GameLayer::update(float delta_time)
{
  DC_PROFILE_SCOPE("GameLayer::update()");
  systems_context_.update(delta_time);

  return false;
}

bool GameLayer::render()
{
  DC_PROFILE_SCOPE("GameLayer::render()");

  SceneRenderInfo scene_render_info{};
  ViewRenderInfo  view_render_info{};
  systems_context_.render(scene_render_info, view_render_info);

  const auto window = Engine::instance()->window();
  view_render_info.set_viewport_info({0, 0, window->width(), window->height()});

  renderer_->render(scene_render_info, view_render_info);

  return false;
}

std::shared_ptr<SceneRenderer> GameLayer::renderer() const { return renderer_; }

SystemsContext *GameLayer::systems_context() { return &systems_context_; }

bool GameLayer::on_event(const Event &event)
{
  DC_PROFILE_SCOPE("GameLayer::on_event()");
  systems_context_.on_event(event);
  return false;
}

void GameLayer::set_scene(std::shared_ptr<SceneAssetHandle> value)
{
  scene_manager_.load_scene(value);
}

std::shared_ptr<SceneAssetHandle> GameLayer::scene() const
{
  return scene_manager_.active_scene();
}

} // namespace dc
