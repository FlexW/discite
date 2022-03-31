#pragma once

#include "skinned_mesh.hpp"
#include "skinned_mesh_asset.hpp"

namespace dc
{

struct SkinnedMeshComponent
{
  std::shared_ptr<SkinnedMeshAssetHandle> skinned_mesh_{};

  SkinnedMeshComponent() = default;

  SkinnedMeshComponent(SkinnedMeshComponent &&other)
      : skinned_mesh_{std::move(other.skinned_mesh_)}
  {
    skinned_mesh_ = nullptr;
  }

  SkinnedMeshComponent(const SkinnedMeshComponent &other)
      : skinned_mesh_{other.skinned_mesh_}
  {
  }

  SkinnedMeshComponent &operator=(SkinnedMeshComponent &&other)
  {
    skinned_mesh_ = std::move(other.skinned_mesh_);
    skinned_mesh_ = nullptr;
    return *this;
  }

  SkinnedMeshComponent &operator=(const SkinnedMeshComponent &other)
  {
    skinned_mesh_ = other.skinned_mesh_;
    return *this;
  }

  void save(FILE *file) const;
  void read(FILE *file);
};

} // namespace dc
