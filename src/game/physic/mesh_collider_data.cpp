#include "mesh_collider_data.hpp"
#include "defer.hpp"
#include "serialization.hpp"

namespace dc
{

void SubmeshColliderData::save(FILE *file) const
{
  write_vector(file, data_);
  write_value(file, transform_);
}

void SubmeshColliderData::read(FILE *file)
{
  read_vector(file, data_);
  read_value(file, transform_);
}

void MeshColliderData::save(const std::filesystem::path &file_path,
                            const AssetDescription &asset_description) const
{
  const auto file = std::fopen(file_path.string().c_str(), "wb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  asset_description.write(file);
  write_value(file,
              static_cast<std::uint8_t>(
                  collider_type_ == MeshColliderType::Convex ? 1 : 0));
  write_value(file, static_cast<std::uint64_t>(sub_meshes_.size()));
  for (const auto &sub_mesh : sub_meshes_)
  {
    sub_mesh.save(file);
  }
}

AssetDescription MeshColliderData::read(const std::filesystem::path &file_path)
{
  const auto file = std::fopen(file_path.string().c_str(), "rb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  AssetDescription asset_description;
  asset_description.read(file);

  std::uint8_t collider_type{};
  read_value(file, collider_type);
  if (collider_type == 0)
  {
    collider_type_ = MeshColliderType::Triangle;
  }
  else
  {
    collider_type_ = MeshColliderType::Convex;
  }

  std::uint64_t sub_meshes_count{};
  read_value(file, sub_meshes_count);
  for (std::uint64_t i = 0; i < sub_meshes_count; ++i)
  {
    SubmeshColliderData sub_mesh{};
    sub_mesh.read(file);
    sub_meshes_.push_back(std::move(sub_mesh));
  }

  return asset_description;
}

} // namespace dc
