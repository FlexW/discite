#include "asset_importer_manager.hpp"
#include "assert.hpp"
#include "log.hpp"

#include <stdexcept>

namespace dc
{

void AssetImporterManager::register_asset_importer(
    const std::string   &file_extension,
    const AssetImporter &asset_importer)
{
  // check if asset importer was already registered
  const auto iter = asset_importer_cache_.find(file_extension);
  if (iter != asset_importer_cache_.end())
  {
    DC_FAIL("Asset importer already registered");
    return;
  }

  // insert asset importer
  asset_importer_cache_[file_extension] = asset_importer;
}

void AssetImporterManager::import_asset(
    const std::filesystem::path &asset_file_path)
{
  DC_LOG_DEBUG("Trying to import asset {}", asset_file_path.string());

  // search for asset importer
  const auto file_extension = asset_file_path.extension().string();
  const auto iter           = asset_importer_cache_.find(file_extension);
  if (iter == asset_importer_cache_.end())
  {
    throw std::runtime_error(
        "Could not find assert importer for asset of type " + file_extension);
  }

  // import asset
  iter->second(asset_file_path);
}

} // namespace dc
