#pragma once

#include "asset_handle.hpp"
#include "material.hpp"

#include <filesystem>
#include <memory>

namespace dc
{

class MaterialAssetHandle : public AssetHandle
{
public:
  MaterialAssetHandle(const std::filesystem::path &file_path,
                      const Asset                 &asset);

  bool is_ready() const override;

  std::shared_ptr<Material> get() const;

private:
  std::shared_ptr<Material> material_;
};

std::shared_ptr<AssetHandle>
material_asset_loader(const std::filesystem::path &file_path,
                      const Asset                 &asset);

} // namespace dc
