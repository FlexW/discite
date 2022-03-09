#include "sky_component.hpp"
#include "engine.hpp"
#include "env_map_asset.hpp"
#include "serialization.hpp"

namespace dc
{

void SkyComponent::save(FILE *file) const
{
  std::string asset_name;
  if (environment_)
  {
    asset_name = environment_->asset().id();
  }
  write_string(file, asset_name);
}

void SkyComponent::read(FILE *file)
{
  std::string asset_name;
  read_string(file, asset_name);
  if (!asset_name.empty())
  {
    environment_ = std::dynamic_pointer_cast<EnvMapAssetHandle>(
        Engine::instance()->asset_cache()->load_asset(Asset{asset_name}));
  }
  else
  {
    environment_ = nullptr;
  }
}

} // namespace dc
