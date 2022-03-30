#pragma once

#include "asset.hpp"
#include "asset_handle.hpp"
#include "skinned_mesh.hpp"

namespace dc
{

class SkinnedMeshAssetHandle : public AssetHandle
{
public:
  SkinnedMeshAssetHandle(const std::filesystem::path &file_path,
                         const Asset                 &asset);

  bool is_ready() const override;

  std::shared_ptr<SkinnedMesh> get() const;

private:
  std::shared_ptr<SkinnedMesh> skinned_mesh_{};
};

std::shared_ptr<AssetHandle>
skinned_mesh_asset_loader(const std::filesystem::path &file_path,
                          const Asset                 &asset);

} // namespace dc
