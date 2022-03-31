#pragma once

#include "defer.hpp"
#include "mesh.hpp"
#include "skeleton.hpp"
#include "skinned_mesh.hpp"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace dc
{

std::filesystem::path normalize_path(const std::filesystem::path &path);

void write_string(FILE *file, const std::string &str);
void read_string(FILE *file, std::string &str);

template <typename T> void write_value(FILE *file, const T &value)
{
  assert(file);
  std::fwrite(&value, sizeof(T), 1, file);
}

template <typename T> void read_value(FILE *file, T &value)
{
  assert(file);
  std::fread(&value, sizeof(T), 1, file);
}

template <typename T> void write_vector(FILE *file, const std::vector<T> &value)
{
  assert(file);
  const auto count = value.size();
  std::fwrite(&count, sizeof(std::uint64_t), 1, file);
  std::fwrite(value.data(), sizeof(T), value.size(), file);
}

template <typename T>
void write_vector_complex(FILE *file, const std::vector<T> &value)
{
  assert(file);
  const auto count = value.size();
  std::fwrite(&count, sizeof(std::uint64_t), 1, file);
  for (std::size_t i = 0; i < value.size(); ++i)
  {
    value[i].save(file);
  }
}

template <typename T> void read_vector(FILE *file, std::vector<T> &value)
{
  assert(file);
  std::uint64_t count{};
  std::fread(&count, sizeof(std::uint64_t), 1, file);
  if (count > 0)
  {
    value.resize(count);
    std::fread(value.data(), sizeof(T), count, file);
  }
  else
  {
    value = {};
  }
}

template <typename T>
void read_vector_complex(FILE *file, std::vector<T> &value)
{
  assert(file);
  std::uint64_t count{};
  std::fread(&count, sizeof(std::uint64_t), 1, file);
  if (count > 0)
  {
    value.resize(count);
    for (std::uint64_t i = 0; i < value.size(); ++i)
    {
      T v{};
      v.read(file);
      value[i] = std::move(v);
    }
  }
  else
  {
    value = {};
  }
}

struct AssetDescription
{
  std::uint32_t magic_value_{0xdeadbeef};
  std::uint32_t version_{0};
  std::string   original_file_;

  void write(FILE *file) const;
  void read(FILE *file);
};

struct TextureDescription
{
  std::vector<std::uint8_t> data_;

  void             save(const std::filesystem::path &file_path,
                        const AssetDescription      &asset_description) const;
  AssetDescription read(const std::filesystem::path &file_path);
};

struct MaterialDescription
{
  std::string albedo_texture_name_;
  glm::vec4   albedo_color_{1.0f};

  std::string roughness_texture_name_;
  glm::vec4   roughness_{1.0f};

  std::string ambient_occlusion_texture_name_;

  std::string emissive_texture_name_;
  glm::vec4   emissive_color_{0.0f};

  std::string normal_texture_name_;

  float transparency_factor_{1.0f};
  float alpha_test_{0.0f};

  float metallic_factor_{0.0f};

  void save(const std::filesystem::path &file_path,
            const AssetDescription      &asset_description) const;

  AssetDescription read(const std::filesystem::path &file_path);
};

struct SubMeshDescription
{
  std::vector<Vertex>        vertices_;
  std::vector<std::uint32_t> indices_;
  std::string                material_name_;

  void save(FILE *file) const;
  void read(FILE *file);
};

struct MeshDescription
{
  std::vector<SubMeshDescription> sub_meshes_;

  void             save(const std::filesystem::path &file_path,
                        const AssetDescription      &asset_description) const;
  AssetDescription read(const std::filesystem::path &file_path);
};

struct SkinnedSubMeshDescription
{
  std::vector<SkinnedVertex> vertices_;
  std::vector<std::uint32_t> indices_;
  std::string                material_name_;

  void save(FILE *file) const;
  void read(FILE *file);
};

struct SkinnedMeshDescription
{
  std::vector<SkinnedSubMeshDescription> sub_meshes_;
  std::shared_ptr<Skeleton> skeleton_{std::make_shared<Skeleton>()};

  void             save(const std::filesystem::path &file_path,
                        const AssetDescription      &asset_description) const;
  AssetDescription read(const std::filesystem::path &file_path);
};

struct EnvironmentMapDescription
{
  std::vector<std::uint8_t> env_map_data_;

  void             save(const std::filesystem::path &file_path,
                        const AssetDescription      &asset_description) const;
  AssetDescription read(const std::filesystem::path &file_path);
};

} // namespace dc
