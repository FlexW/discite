#pragma once

#include "env_map_asset.hpp"

namespace dc
{

struct SkyComponent
{

  std::shared_ptr<EnvMapAssetHandle> environment_{};

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
