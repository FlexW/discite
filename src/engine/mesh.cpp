#include "mesh.hpp"
#include "gl.hpp"
#include "gl_index_buffer.hpp"
#include "gl_texture.hpp"
#include "gl_vertex_buffer.hpp"
#include "log.hpp"
#include "material_asset.hpp"
#include "math.hpp"
#include "serialization.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>

#include <cstddef>
#include <memory>
#include <stdexcept>

namespace dc
{

void SubMeshDescription::save(FILE *file) const
{
  write_vector(file, vertices_);
  write_vector(file, indices_);
  write_string(file, material_name_);
}

void SubMeshDescription::read(FILE *file)
{
  read_vector(file, vertices_);
  read_vector(file, indices_);
  read_string(file, material_name_);
}

void MeshDescription::save(const std::filesystem::path &file_path,
                           const AssetDescription      &asset_description) const
{
  const auto file = std::fopen(file_path.string().c_str(), "wb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  asset_description.write(file);

  write_value(file, static_cast<std::uint64_t>(sub_meshes_.size()));
  for (const auto &sub_mesh : sub_meshes_)
  {
    sub_mesh.save(file);
  }
}

AssetDescription MeshDescription::read(const std::filesystem::path &file_path)
{
  const auto file = std::fopen(file_path.string().c_str(), "rb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  AssetDescription asset_description;
  asset_description.read(file);

  std::uint64_t sub_meshes_count{};
  read_value(file, sub_meshes_count);
  for (std::uint64_t i = 0; i < sub_meshes_count; ++i)
  {
    SubMeshDescription sub_mesh_description{};
    sub_mesh_description.read(file);
    sub_meshes_.push_back(std::move(sub_mesh_description));
  }

  return asset_description;
}

SubMesh::SubMesh(std::unique_ptr<GlVertexArray>       vertex_array,
                 std::shared_ptr<MaterialAssetHandle> material)
    : vertex_array_{std::move(vertex_array)},
      material_{material}
{
}

SubMesh::SubMesh(SubMesh &&other)
    : vertex_array_{std::move(other.vertex_array_)},
      material_{std::move(other.material_)}
{
  other.vertex_array_ = nullptr;
  other.material_     = nullptr;
}

void SubMesh::operator=(SubMesh &&other)
{
  vertex_array_       = std::move(other.vertex_array_);
  other.vertex_array_ = nullptr;
  material_           = std::move(other.material_);
  other.material_     = nullptr;
}

GlVertexArray *SubMesh::vertex_array() const { return vertex_array_.get(); }

Material *SubMesh::material() const
{

  if (!material_->is_ready())
  {
    return nullptr;
  }

  return material_->get().get();
}

Mesh::Mesh(Mesh &&other) { meshes_ = std::move(other.meshes_); }

void Mesh::operator=(Mesh &&other) { meshes_ = std::move(other.meshes_); }

std::vector<SubMesh *> Mesh::meshes() const
{

  std::vector<SubMesh *> raw_meshes(meshes_.size());
  for (std::size_t i = 0; i < raw_meshes.size(); ++i)
  {
    raw_meshes[i] = meshes_[i].get();
  }
  return raw_meshes;
}

void Mesh::set_meshes(std::vector<std::unique_ptr<SubMesh>> meshes)
{
  meshes_ = std::move(meshes);
}

void Mesh::set_description(MeshDescription value)
{
  mesh_description_ = std::move(value);
}

MeshDescription Mesh::get_description() const { return mesh_description_; }

} // namespace dc
