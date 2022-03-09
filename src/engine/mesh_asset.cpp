#include "mesh_asset.hpp"
#include "asset.hpp"
#include "asset_handle.hpp"
#include "engine.hpp"
#include "gl_index_buffer.hpp"
#include "gl_vertex_buffer.hpp"
#include "log.hpp"
#include "material_asset.hpp"
#include "mesh.hpp"
#include "serialization.hpp"

#include <memory>
#include <stdexcept>

namespace dc
{

MeshAssetHandle::MeshAssetHandle(const std::filesystem::path &file_path,
                                 const Asset                 &asset)
    : AssetHandle(asset)
{
  try
  {
    MeshDescription mesh_description{};
    mesh_description.read(file_path);

    std::vector<std::unique_ptr<Mesh>> meshes;
    for (const auto &sub_mesh : mesh_description.sub_meshes_)
    {
      const auto asset_cache = Engine::instance()->asset_cache();
      const auto material    = std::dynamic_pointer_cast<MaterialAssetHandle>(
          asset_cache->load_asset(Asset{sub_mesh.material_name_}));

      const auto index_buffer = std::make_shared<GlIndexBuffer>();
      index_buffer->set_data(sub_mesh.indices_);

      const auto           vertex_buffer = std::make_shared<GlVertexBuffer>();
      GlVertexBufferLayout layout;
      layout.push_float(3); // position
      layout.push_float(3); // normal
      layout.push_float(3); // tangent
      layout.push_float(3); // bitanget
      layout.push_float(2); // tex coords
      vertex_buffer->set_data(sub_mesh.vertices_, layout);

      auto vertex_array = std::make_unique<GlVertexArray>();
      vertex_array->add_vertex_buffer(vertex_buffer);
      vertex_array->set_index_buffer(index_buffer);

      auto mesh = std::make_unique<Mesh>(std::move(vertex_array), material);
      meshes.push_back(std::move(mesh));
    }

    model_ = std::make_shared<Model>();
    model_->set_meshes(std::move(meshes));
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN() << "Could not load mesh asset " << file_path.string() << ": "
               << error.what();
  }
}

bool MeshAssetHandle::is_ready() const { return model_ != nullptr; }

std::shared_ptr<Model> MeshAssetHandle::get() const { return model_; }

std::shared_ptr<AssetHandle>
mesh_asset_loader(const std::filesystem::path &file_path, const Asset &asset)
{
  return std::make_shared<MeshAssetHandle>(file_path, asset);
}

} // namespace dc
