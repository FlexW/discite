#include "env_map_asset.hpp"
#include "asset.hpp"
#include "asset_handle.hpp"
#include "environment_map.hpp"
#include "gl_cube_texture.hpp"
#include "log.hpp"
#include "serialization.hpp"

#include <memory>
#include <stdexcept>

namespace dc
{

EnvMapAssetHandle::EnvMapAssetHandle(const std::filesystem::path &file_path,
                                     const Asset &                asset)
    : AssetHandle(asset)
{
  try
  {
    EnvironmentMapDescription env_map_description{};
    env_map_description.read(file_path);

    if (env_map_description.env_map_data_.empty())
    {
      throw std::runtime_error("Env map " + file_path.string() +
                               " data is empty");
    }

    env_map_ =
        std::make_shared<EnvironmentMap>(asset.id(),
                                         env_map_description.env_map_data_);
    ;
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not load env map asset {}: {}",
                file_path.string(),
                error.what());
  }
}

bool EnvMapAssetHandle::is_ready() const { return env_map_ != nullptr; }

std::shared_ptr<EnvironmentMap> EnvMapAssetHandle::get() const
{
  return env_map_;
}

std::shared_ptr<AssetHandle>
env_map_asset_loader(const std::filesystem::path &file_path, const Asset &asset)
{
  return std::make_shared<EnvMapAssetHandle>(file_path, asset);
}

} // namespace dc
