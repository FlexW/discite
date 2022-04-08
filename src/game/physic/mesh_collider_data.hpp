#pragma once

#include "asset_description.hpp"
#include "math.hpp"

#include <cstdint>
#include <filesystem>
#include <vector>

namespace dc
{

enum class MeshColliderType
{
  Triangle,
  Convex,
};

struct SubmeshColliderData
{
  std::vector<std::uint8_t> data_;
  glm::mat4                 transform_;

  void save(FILE *file) const;
  void read(FILE *file);
};

struct MeshColliderData
{
  MeshColliderType collider_type_{MeshColliderType::Triangle};

  std::vector<SubmeshColliderData> sub_meshes_;

  void             save(const std::filesystem::path &file_path,
                        const AssetDescription      &asset_description) const;
  AssetDescription read(const std::filesystem::path &file_path);
};

} // namespace dc
