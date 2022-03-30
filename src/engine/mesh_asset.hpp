#pragma once

#include "asset.hpp"
#include "asset_handle.hpp"
#include "mesh.hpp"

#include <filesystem>
#include <memory>

namespace dc
{

class MeshAssetHandle : public AssetHandle
{
public:
  MeshAssetHandle(const std::filesystem::path &file_path, const Asset &asset);

  bool is_ready() const override;

  std::shared_ptr<Mesh> get() const;

private:
  std::shared_ptr<Mesh> model_{};
};

std::shared_ptr<AssetHandle>
mesh_asset_loader(const std::filesystem::path &file_path, const Asset &asset);

} // namespace dc
