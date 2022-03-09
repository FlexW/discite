#include "asset_handle.hpp"
#include "asset.hpp"

namespace dc
{

AssetHandle::AssetHandle(const Asset &asset) : asset_{asset} {}

Asset AssetHandle::asset() const { return asset_; }

} // namespace dc
