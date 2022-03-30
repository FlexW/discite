#include "skinned_mesh_asset.hpp"
#include "asset_handle.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "serialization.hpp"
#include "skeleton.hpp"
#include "skinned_mesh.hpp"

#include <memory>
#include <stdexcept>

namespace dc
{

SkinnedMeshAssetHandle::SkinnedMeshAssetHandle(
    const std::filesystem::path &file_path,
    const Asset                 &asset)
    : AssetHandle{asset}
{
  try
  {
    SkinnedMeshDescription skinned_mesh_desc{};
    skinned_mesh_desc.read(file_path);

    std::vector<std::unique_ptr<SkinnedSubMesh>> sub_meshes;
    for (const auto &sub_mesh : skinned_mesh_desc.sub_meshes_)
    {
      const auto asset_cache = Engine::instance()->asset_cache();
      const auto material    = std::dynamic_pointer_cast<MaterialAssetHandle>(
          asset_cache->load_asset(Asset{sub_mesh.material_name_}));

      const auto index_buffer =
          std::make_shared<GlIndexBuffer>(sub_mesh.indices_);

      GlVertexBufferLayout layout;
      layout.push_float(3); // position
      layout.push_float(3); // normal
      layout.push_float(3); // tangent
      layout.push_float(3); // bitanget
      layout.push_int(4);   // bones
      layout.push_float(4); // bone weights
      layout.push_float(2); // tex coords
      const auto vertex_buffer =
          std::make_shared<GlVertexBuffer>(sub_mesh.vertices_, layout);

      auto vertex_array = std::make_unique<GlVertexArray>();
      vertex_array->add_vertex_buffer(vertex_buffer);
      vertex_array->set_index_buffer(index_buffer);

      auto mesh =
          std::make_unique<SkinnedSubMesh>(std::move(vertex_array), material);
      sub_meshes.push_back(std::move(mesh));
    }

    skinned_mesh_ =
        std::make_shared<SkinnedMesh>(std::move(skinned_mesh_desc.skeleton_),
                                      std::move(sub_meshes));
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not load skinned mesh asset {}: {}",
                file_path.string(),
                error.what());
  }
}

bool SkinnedMeshAssetHandle::is_ready() const
{
  return skinned_mesh_ != nullptr;
}

std::shared_ptr<SkinnedMesh> SkinnedMeshAssetHandle::get() const
{
  return skinned_mesh_;
}

std::shared_ptr<AssetHandle>
skinned_mesh_asset_loader(const std::filesystem::path &file_path,
                          const Asset                 &asset)
{
  return std::make_shared<SkinnedMeshAssetHandle>(file_path, asset);
}

} // namespace dc
