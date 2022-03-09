#include "scene_asset.hpp"
#include "asset_handle.hpp"
#include "log.hpp"
#include "scene.hpp"

#include <stdexcept>

namespace dc
{

SceneAssetHandle::SceneAssetHandle(const std::filesystem::path &file_path,
                                   const Asset                 &asset)
    : AssetHandle{asset},
      file_path_{file_path}
{
  try
  {
    scene_ = Scene::create();
    asset_description_ = scene_->read(file_path_);
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not load scene asset {}", file_path.string());
  }
}

bool SceneAssetHandle::is_ready() const { return scene_ != nullptr; }

void SceneAssetHandle::save() const
{
  if (scene_)
  {
    scene_->save(file_path_, asset_description_);
  }
}

std::shared_ptr<Scene> SceneAssetHandle::get() const { return scene_; }

std::shared_ptr<AssetHandle>
scene_asset_loader(const std::filesystem::path &file_path, const Asset &asset)
{
  return std::make_shared<SceneAssetHandle>(file_path, asset);
}

} // namespace dc
