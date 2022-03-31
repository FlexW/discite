#pragma once

#include "animation_state.hpp"
#include "skinned_mesh.hpp"
#include "skinned_mesh_asset.hpp"

#include <memory>

namespace dc
{

struct SkinnedMeshComponent
{
public:
  void set_skinned_mesh_asset(std::shared_ptr<SkinnedMeshAssetHandle> value);
  std::shared_ptr<SkinnedMeshAssetHandle> skinned_mesh_asset() const;

  AnimationState *animation_state() const;

  void                                  save(FILE *file) const;
  void                                  read(FILE *file);

private:
  std::shared_ptr<SkinnedMeshAssetHandle> skinned_mesh_{};
  std::shared_ptr<AnimationState>         animation_state_{};
};

} // namespace dc
