#pragma once

#include "asset.hpp"
#include "asset_handle.hpp"
#include "gl_texture.hpp"

#include <filesystem>
#include <memory>

namespace dc
{

class TextureAssetHandle : public AssetHandle
{
public:
  TextureAssetHandle(const std::filesystem::path &file_path,
                     const Asset                 &asset);

  bool is_ready() const override;

  std::shared_ptr<GlTexture> get() const;

private:
  std::shared_ptr<GlTexture> texture_{};
};

std::shared_ptr<AssetHandle>
texture_asset_loader(const std::filesystem::path &file_path,
                     const Asset                 &asset);

} // namespace dc
