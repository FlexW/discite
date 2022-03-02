#pragma once

#include "env_map_asset.hpp"

struct SkyComponent
{

  std::shared_ptr<EnvMapAssetHandle> environment_{};

  void save(FILE *file) const;
  void read(FILE *file);
};
