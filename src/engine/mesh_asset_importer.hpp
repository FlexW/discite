#pragma once

#include "asset_importer_manager.hpp"
#include "serialization.hpp"

#include <assimp/scene.h>

#include <set>

struct MeshImportData
{
  std::filesystem::path base_path_;
  std::string           mesh_name_;

  MeshDescription                                      mesh_;
  std::set<std::string>                                materials_;
  std::unordered_map<std::string, TextureDescription>  textures_;
};

void do_import_mesh(const aiScene  *ai_scene,
                    const aiMesh   *ai_mesh,
                    aiMatrix4x4    &transform,
                    MeshImportData &import_data);

void import_mesh_asset(const std::filesystem::path &file_path,
                       const std::string           &name);
