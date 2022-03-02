#include "asset_handle.hpp"
#include "asset.hpp"

AssetHandle::AssetHandle(const Asset &asset) : asset_{asset} {}

Asset AssetHandle::asset() const { return asset_; }
