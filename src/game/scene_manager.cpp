#include "scene_manager.hpp"
#include "assert.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "scene_asset.hpp"
#include "scene_events.hpp"

#include <memory>

namespace dc
{

void SceneManager::init()
{
  is_init_ = true;

  if (active_scene_)
  {
    // notify everyone that the new scene was loaded
    const auto scene_loaded_event =
        std::make_shared<SceneLoadedEvent>(active_scene_->get());
    Engine::instance()->event_manager()->queue_event(scene_loaded_event);
  }
}

void SceneManager::load_scene(std::shared_ptr<SceneAssetHandle> new_scene)
{
  if (!new_scene)
  {
    DC_LOG_WARN("Can not load nullptr scene");
    return;
  }

  if (!is_init_)
  {
    // we inform everyone about the new scene only after init
    active_scene_ = new_scene;
    return;
  }

  // unload scene
  const auto old_scene = active_scene_;
  active_scene_        = {};

  // notify everyone that the scene got unloaded
  const auto event_manager = Engine::instance()->event_manager();
  const auto scene_unloaded_event =
      std::make_shared<SceneUnloadedEvent>(old_scene->get());
  event_manager->queue_event(scene_unloaded_event);

  // set the new scene as active
  active_scene_ = new_scene;

  // notify everyone that the new scene was loaded
  const auto scene_loaded_event =
      std::make_shared<SceneLoadedEvent>(active_scene_->get());
  event_manager->queue_event(scene_loaded_event);
}

void SceneManager::load_scene(const std::string &name)
{
  const auto asset_cache = Engine::instance()->asset_cache();
  const auto new_scene   = std::dynamic_pointer_cast<SceneAssetHandle>(
      asset_cache->load_asset(Asset{name}));

  if (!new_scene)
  {
    DC_LOG_WARN("Could not load scene {}", name);
    return;
  }

  load_scene(new_scene);
}

std::shared_ptr<SceneAssetHandle> SceneManager::active_scene() const
{
  return active_scene_;
}

} // namespace dc
