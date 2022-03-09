#include "asset_cache.hpp"
#include "engine.hpp"
#include "log.hpp"

#include <cassert>

namespace dc
{

void AssetCache::register_asset_loader(std::string        asset_type,
                                       const AssetLoader &asset_loader)
{
  const auto iter = asset_loaders_.find(asset_type);
  if (iter != asset_loaders_.end())
  {
    assert(0 && "Asset loader already registered");
    return;
  }
  asset_loaders_[asset_type] = asset_loader;
}

std::shared_ptr<AssetHandle> AssetCache::load_asset(const Asset &asset)
{
  if (asset.type().empty())
  {
    DC_LOG_DEBUG() << "Trying to load an invalid asset";
    return nullptr;
  }
  // check if asset already loaded and if yes, load it from cache
  const auto asset_iter = asset_cache_.find(asset.id());
  if (asset_iter != asset_cache_.end())
  {
    return asset_iter->second;
  }

  // find matching asset loader
  const auto asset_loader_iter = asset_loaders_.find(asset.type());
  if (asset_loader_iter == asset_loaders_.end())
  {
    DC_LOG_WARN() << "No such asset loader " + asset.type();
    return nullptr;
  }

  // load the asset from disk
  const auto asset_handle = asset_loader_iter->second(
      Engine::instance()->base_directory() / asset.id(),
      asset);
  asset_cache_[asset.id()] = asset_handle;
  return asset_handle;
}

} // namespace dc
