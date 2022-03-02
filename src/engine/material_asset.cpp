#include "material_asset.hpp"
#include "asset.hpp"
#include "asset_handle.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "material.hpp"
#include "serialization.hpp"
#include "texture_asset.hpp"

#include <memory>
#include <stdexcept>

MaterialAssetHandle::MaterialAssetHandle(const std::filesystem::path &file_path, const Asset&asset) : AssetHandle{asset}
{
  try
  {
    MaterialDescription material_description{};
    material_description.read(file_path);

    const auto asset_cache = Engine::instance()->asset_cache();
    material_              = std::make_shared<Material>();

    material_->set_albedo_texture(
        std::dynamic_pointer_cast<TextureAssetHandle>(asset_cache->load_asset(
            Asset{material_description.albedo_texture_name_})));
    material_->set_albedo_color(material_description.albedo_color_);

    material_->set_emissive_texture(
        std::dynamic_pointer_cast<TextureAssetHandle>(asset_cache->load_asset(
            Asset{material_description.emissive_texture_name_})));
    material_->set_emissive_color(material_description.emissive_color_);

    material_->set_roughness_texture(
        std::dynamic_pointer_cast<TextureAssetHandle>(asset_cache->load_asset(
            Asset{material_description.roughness_texture_name_})));
    material_->set_roughness(material_description.roughness_);

    material_->set_ambient_occlusion_texture(
        std::dynamic_pointer_cast<TextureAssetHandle>(asset_cache->load_asset(
            Asset{material_description.ambient_occlusion_texture_name_})));

    material_->set_normal_texture(
        std::dynamic_pointer_cast<TextureAssetHandle>(asset_cache->load_asset(
            Asset{material_description.normal_texture_name_})));

    // TODO: Handle transparency
    material_->set_transparent(false);
  }
  catch (const std::runtime_error &error)
  {
    LOG_WARN() << "Could not load material asset " << file_path.string() << ": "
               << error.what();
  }
}

bool MaterialAssetHandle::is_ready() const { return material_ != nullptr; }

std::shared_ptr<Material> MaterialAssetHandle::get() const { return material_; }

std::shared_ptr<AssetHandle>
material_asset_loader(const std::filesystem::path &file_path,
                      const Asset                 &asset)
{
  return std::make_shared<MaterialAssetHandle>(file_path, asset);
}
