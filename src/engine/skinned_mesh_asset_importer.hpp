#pragma once

#include "asset_importer_manager.hpp"
#include "serialization.hpp"

#include <assimp/scene.h>

#include <set>

namespace dc
{

struct SkinnedMeshImportData
{
  std::filesystem::path base_path_;
  std::string           skinned_mesh_name_;

  SkinnedMeshDescription                              skinned_mesh_;
  std::set<std::string>                               materials_;
  std::unordered_map<std::string, TextureDescription> textures_;
};

void do_import_skinned_mesh(const aiScene         *ai_scene,
                            const aiMesh          *ai_mesh,
                            aiMatrix4x4           &transform,
                            SkinnedMeshImportData &import_data);

void import_skinned_mesh_asset(const std::filesystem::path &file_path,
                               const std::string           &name);

} // namespace dc
