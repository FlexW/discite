#pragma once

#include "asset.hpp"
#include "asset_handle.hpp"
#include "scene.hpp"
#include "serialization.hpp"

#include <filesystem>
#include <memory>

class SceneAssetHandle : public AssetHandle
{
public:
  SceneAssetHandle(const std::filesystem::path &file_path, const Asset &asset);

  bool is_ready() const override;

  std::shared_ptr<Scene> get() const;

  void save() const;

private:
  std::filesystem::path  file_path_;
  AssetDescription       asset_description_{};
  std::shared_ptr<Scene> scene_{};
};

std::shared_ptr<AssetHandle>
scene_asset_loader(const std::filesystem::path &file_path, const Asset &asset);
