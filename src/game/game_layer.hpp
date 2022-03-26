#pragma once

#include "layer.hpp"
#include "scene.hpp"
#include "scene_asset.hpp"
#include "scene_manager.hpp"
#include "scene_renderer.hpp"
#include "systems_context.hpp"

#include <memory>

namespace dc
{

class GameLayer : public Layer
{
public:
  GameLayer();

  void register_asset_loaders() override;

  void init() override;
  void shutdown() override;

  bool update(float delta_time) override;
  bool render() override;

  bool on_event(const Event &event) override;

  std::shared_ptr<SceneRenderer> renderer() const;
  SystemsContext                *systems_context();

  void set_scene(std::shared_ptr<SceneAssetHandle> value);
  std::shared_ptr<SceneAssetHandle> scene() const;

private:
  SceneManager   scene_manager_;
  SystemsContext systems_context_;

  std::shared_ptr<SceneRenderer> renderer_{};
};

} // namespace dc
