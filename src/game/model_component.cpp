#include "model_component.hpp"
#include "engine.hpp"
#include "mesh_asset.hpp"
#include "serialization.hpp"

#include <memory>

namespace dc
{

void ModelComponent::save(FILE *file) const
{
  std::string mesh_asset_name;
  if (model_)
  {
    mesh_asset_name = model_->asset().id();
  }
  write_string(file, mesh_asset_name);
}

void ModelComponent::read(FILE *file)
{
  std::string mesh_asset_name;
  read_string(file, mesh_asset_name);
  if (!mesh_asset_name.empty())
  {
    model_ = std::dynamic_pointer_cast<MeshAssetHandle>(
        Engine::instance()->asset_cache()->load_asset(Asset{mesh_asset_name}));
  }
  else
  {
    model_ = nullptr;
  }
}

} // namespace dc
