#include "skinned_mesh_component.hpp"
#include "engine.hpp"
#include "serialization.hpp"

namespace dc
{

void SkinnedMeshComponent::save(FILE *file) const
{
  std::string skinned_mesh_asset_name;
  if (skinned_mesh_)
  {
    skinned_mesh_asset_name = skinned_mesh_->asset().id();
  }
  write_string(file, skinned_mesh_asset_name);
  if (skinned_mesh_ && skinned_mesh_->is_ready())
  {
    const auto skinned_mesh = skinned_mesh_->get();
    write_value(file, skinned_mesh->is_animation_endless());
    write_string(file, skinned_mesh->current_animation_name());
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
      bool is_endless{false};
      read_value(file, is_endless);
      skinned_mesh_->get()->set_animation_endless(is_endless);

      std::string animation_name;
      read_string(file, animation_name);
      skinned_mesh_->get()->play_animation(animation_name);
    }
  }
  else
  {
    skinned_mesh_ = nullptr;
  }
}

} // namespace dc
