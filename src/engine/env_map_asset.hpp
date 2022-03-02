#pragma once

#include "asset.hpp"
#include "asset_handle.hpp"
#include "environment_map.hpp"

#include <filesystem>
#include <memory>

class EnvMapAssetHandle : public AssetHandle
{
public:
  EnvMapAssetHandle(const std::filesystem::path &file_path, const Asset &asset);

  bool is_ready() const override;

  std::shared_ptr<EnvionmentMap> get() const;

private:
  std::shared_ptr<EnvionmentMap> env_map_{};
};

std::shared_ptr<AssetHandle>
env_map_asset_loader(const std::filesystem::path &file_path,
                     const Asset                 &asset);
