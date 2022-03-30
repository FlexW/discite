#pragma once

#include "skinned_mesh_asset.hpp"

namespace dc
{

struct SkinnedMeshComponent
{
  std::shared_ptr<SkinnedMeshAssetHandle> skinned_mesh_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
