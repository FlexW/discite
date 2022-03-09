#pragma once

#include "asset.hpp"
#include "asset_handle.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>

namespace dc
{

using AssetLoader =
    std::function<std::shared_ptr<AssetHandle>(const std::filesystem::path &,
                                               const Asset &asset)>;

class AssetCache
{
public:
  void register_asset_loader(std::string        asset_type,
                             const AssetLoader &asset_loader);

  std::shared_ptr<AssetHandle> load_asset(const Asset &asset);

private:
  std::unordered_map<std::string, AssetLoader>                  asset_loaders_;
  std::unordered_map<std::string, std::shared_ptr<AssetHandle>> asset_cache_;
};

} // namespace dc
