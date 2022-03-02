#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>

using AssetImporter =
    std::function<void(const std::filesystem::path &file_path)>;

class AssetImporterManager
{
public:
  void register_asset_importer(const std::string   &file_extension,
                               const AssetImporter &asset_importer);

  void import_asset(const std::filesystem::path &asset_file_path);

private:
  std::unordered_map<std::string, AssetImporter> asset_importer_cache_;
};
