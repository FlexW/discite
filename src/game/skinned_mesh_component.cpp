#include "skinned_mesh_component.hpp"
#include "animation_state.hpp"
#include "engine.hpp"
#include "serialization.hpp"
#include "skinned_mesh_asset.hpp"

#include <memory>

namespace dc
{

void SkinnedMeshComponent::set_skinned_mesh_asset(
    std::shared_ptr<SkinnedMeshAssetHandle> value)
{
  skinned_mesh_ = value;
  DC_ASSERT(skinned_mesh_->is_ready(), "Asset not ready!");
  animation_state_ =
      std::make_unique<AnimationState>(skinned_mesh_->get()->skeleton());
}

std::shared_ptr<SkinnedMeshAssetHandle>
SkinnedMeshComponent::skinned_mesh_asset() const
{
  return skinned_mesh_;
}

AnimationState *SkinnedMeshComponent::animation_state() const
{
  return animation_state_.get();
}

void SkinnedMeshComponent::save(FILE *file) const
{
  std::string skinned_mesh_asset_name;
  if (skinned_mesh_)
  {
    skinned_mesh_asset_name = skinned_mesh_->asset().id();
  }
  write_string(file, skinned_mesh_asset_name);
  if (animation_state_)
  {
    animation_state_->save(file);
  }
}

void SkinnedMeshComponent::read(FILE *file)
{
  std::string skinned_mesh_asset_name;
  read_string(file, skinned_mesh_asset_name);
  if (!skinned_mesh_asset_name.empty())
  {
    skinned_mesh_ = std::dynamic_pointer_cast<SkinnedMeshAssetHandle>(
        Engine::instance()->asset_cache()->load_asset(
            Asset{skinned_mesh_asset_name}));

    DC_ASSERT(skinned_mesh_->is_ready(), "Asset not ready");
    if (skinned_mesh_->is_ready())
    {
      animation_state_ =
          std::make_unique<AnimationState>(skinned_mesh_->get()->skeleton());
      animation_state_->read(file);
    }
  }
  else
  {
    skinned_mesh_ = nullptr;
  }
}

} // namespace dc
