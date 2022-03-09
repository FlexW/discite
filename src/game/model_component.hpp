#pragma once

#include "mesh_asset.hpp"

namespace dc
{

struct ModelComponent
{
  std::shared_ptr<MeshAssetHandle> model_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
