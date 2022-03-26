#pragma once

#include "scene_asset.hpp"

#include <memory>

namespace dc
{

class SceneManager
{
public:
  void init();
  void load_scene(const std::string &name);
  void load_scene(std::shared_ptr<SceneAssetHandle> new_scene);

  std::shared_ptr<SceneAssetHandle> active_scene() const;

private:
  bool is_init_{false};

  std::shared_ptr<SceneAssetHandle> active_scene_{};
};

} // namespace dc
