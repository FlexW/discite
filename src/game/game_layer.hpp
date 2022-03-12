#pragma once

#include "layer.hpp"
#include "scene.hpp"
#include "scene_asset.hpp"
#include "scene_renderer.hpp"

#include <memory>

namespace dc
{

class GameLayer : public Layer
{
public:
  void register_asset_loaders() override;

  void init() override;
  void shutdown() override;
  void update(float delta_time) override;
  void render() override;

  bool on_event(const Event &event) override;

  std::shared_ptr<SceneRenderer> renderer() const;

  void set_scene(std::shared_ptr<SceneAssetHandle> value);
  std::shared_ptr<SceneAssetHandle> scene() const;

private:
  bool is_init_{false};

  std::shared_ptr<SceneAssetHandle> scene_{};
  std::shared_ptr<SceneRenderer>    renderer_;
};

} // namespace dc
