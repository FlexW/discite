#include "serialization.hpp"
#include "environment_map.hpp"
#include "skeleton.hpp"

#include <cstdint>

namespace dc
{

std::filesystem::path normalize_path(const std::filesystem::path &path)
{
  // TODO: Implement so that Engine base path is removed from path
  return path;
}

void write_string(FILE *file, const std::string &str)
{
  assert(file);
  const auto length = str.size();
  std::fwrite(&length, sizeof(std::uint32_t), 1, file);
  if (length > 0)
  {
    std::fwrite(str.c_str(), sizeof(char), length, file);
  }
}

void read_string(FILE *file, std::string &str)
{
  assert(file);
  std::uint32_t length{};
  std::fread(&length, sizeof(std::uint32_t), 1, file);
  if (length > 0)
  {
    std::vector<char> data(length + 1);
    data[data.size() - 1] = '\0';
    std::fread(data.data(), sizeof(char), length, file);
    str = data.data();
  }
  else
  {
    str = "";
  }
}

void write_string(std::ofstream &file, const std::string &str)
{
  const auto length = str.size();
  file.write(reinterpret_cast<const char *>(&length), sizeof(std::uint32_t));
  if (length > 0)
  {
    file.write(reinterpret_cast<const char *>(str.data()),
               sizeof(char) * length);
  }
}

void read_string(std::ifstream &file, std::string &str)
{
  std::uint32_t length{};
  file.read(reinterpret_cast<char *>(&length), sizeof(std::uint32_t));
  if (length > 0)
  {
    std::vector<char> data(length + 1);
    data[data.size() - 1] = '\0';
    file.read(data.data(), sizeof(char) * length);
    str = data.data();
  }
  else
  {
    str = "";
  }
}

void TextureDescription::save(const std::filesystem::path &file_path,
                              const AssetDescription &asset_description) const
{
  const auto file = std::fopen(file_path.string().c_str(), "wb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  asset_description.write(file);
  write_vector(file, data_);
}

AssetDescription
TextureDescription::read(const std::filesystem::path &file_path)
{
  const auto file = std::fopen(file_path.string().c_str(), "rb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  AssetDescription asset_description;
  asset_description.read(file);

  read_vector(file, data_);

  return asset_description;
}

void MaterialDescription::save(const std::filesystem::path &file_path,
                               const AssetDescription &asset_description) const
{
  const auto file = std::fopen(file_path.string().c_str(), "wb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  asset_description.write(file);
  write_string(file, albedo_texture_name_);
  write_value(file, albedo_color_);
  write_string(file, roughness_texture_name_);
  write_value(file, roughness_);
  write_string(file, ambient_occlusion_texture_name_);
  write_string(file, emissive_texture_name_);
  write_value(file, emissive_color_);
  write_string(file, normal_texture_name_);
  write_value(file, transparency_factor_);
  write_value(file, alpha_test_);
  write_value(file, metallic_factor_);
}

AssetDescription
MaterialDescription::read(const std::filesystem::path &file_path)
{
  const auto file = std::fopen(file_path.string().c_str(), "rb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  AssetDescription asset_description;
  asset_description.read(file);

  read_string(file, albedo_texture_name_);
  read_value(file, albedo_color_);
  read_string(file, roughness_texture_name_);
  read_value(file, roughness_);
  read_string(file, ambient_occlusion_texture_name_);
  read_string(file, emissive_texture_name_);
  read_value(file, emissive_color_);
  read_string(file, normal_texture_name_);
  read_value(file, transparency_factor_);
  read_value(file, alpha_test_);
  read_value(file, metallic_factor_);

  return asset_description;
}

void SkinnedSubMeshDescription::save(FILE *file) const
{
  write_vector(file, vertices_);
  write_vector(file, indices_);
  write_string(file, material_name_);
}

void SkinnedSubMeshDescription::read(FILE *file)
{
  read_vector(file, vertices_);
  read_vector(file, indices_);
  read_string(file, material_name_);
}

void SkinnedMeshDescription::save(
    const std::filesystem::path &file_path,
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
  skeleton_->save(file);
}

AssetDescription
SkinnedMeshDescription::read(const std::filesystem::path &file_path)
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
    SkinnedSubMeshDescription sub_mesh_description{};
    sub_mesh_description.read(file);
    sub_meshes_.push_back(std::move(sub_mesh_description));
  }
  skeleton_->read(file);

  return asset_description;
}

void EnvironmentMapDescription::save(
    const std::filesystem::path &file_path,
    const AssetDescription      &asset_description) const
{
  const auto file = std::fopen(file_path.string().c_str(), "wb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  asset_description.write(file);

  write_vector(file, env_map_data_);
}

AssetDescription
EnvironmentMapDescription::read(const std::filesystem::path &file_path)
{
  const auto file = std::fopen(file_path.string().c_str(), "rb");
  if (!file)
  {
    throw std::runtime_error{"Could not open file " + file_path.string()};
  }
  defer(std::fclose(file));

  AssetDescription asset_description;
  asset_description.read(file);

  read_vector(file, env_map_data_);

  return asset_description;
}

} // namespace dc
