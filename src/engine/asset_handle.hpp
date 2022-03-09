#pragma once

#include "asset.hpp"

#include <filesystem>

namespace dc
{

class AssetHandle
{
public:
  AssetHandle(const Asset &asset);
  virtual ~AssetHandle() = default;

  virtual bool is_ready() const = 0;

  Asset asset() const;

private:
  std::filesystem::path file_path_;
  Asset asset_;
};

} // namespace dc
