#include "texture_asset.hpp"
#include "asset.hpp"
#include "asset_handle.hpp"
#include "defer.hpp"
#include "gl_texture.hpp"
#include "log.hpp"
#include "serialization.hpp"
#include "stb_image.h"
#include <filesystem>
#include <stdexcept>

namespace dc
{

TextureAssetHandle::TextureAssetHandle(const std::filesystem::path &file_path,
                                       const Asset                 &asset)
    : AssetHandle(asset)
{
  try
  {
    TextureDescription texture_description{};
    texture_description.read(file_path);

    int        width{};
    int        height{};
    int        channels_count{};
    const auto loaded_data =
        stbi_load_from_memory(texture_description.data_.data(),
                              texture_description.data_.size(),
                              &width,
                              &height,
                              &channels_count,
                              0);
    if (!loaded_data)
    {
      throw std::runtime_error("Could not load texture " + file_path.string());
    }
    defer(stbi_image_free(loaded_data));

    texture_ = std::make_shared<GlTexture>();
    texture_->set_data(loaded_data, width, height, channels_count, true);
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not load texture asset {}: {}",
                file_path.string(),
                error.what());
  }
}

bool TextureAssetHandle::is_ready() const { return texture_ != nullptr; }

std::shared_ptr<AssetHandle>
texture_asset_loader(const std::filesystem::path &file_path, const Asset &asset)
{
  return std::make_shared<TextureAssetHandle>(file_path, asset);
}

std::shared_ptr<GlTexture> TextureAssetHandle::get() const { return texture_; }

} // namespace dc
