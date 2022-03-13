#include "scene_asset_importer.hpp"
#include "assimp/matrix4x4.h"
#include "engine.hpp"
#include "entity.hpp"
#include "importer.hpp"
#include "log.hpp"
#include "mesh_asset.hpp"
#include "mesh_asset_importer.hpp"
#include "model_component.hpp"
#include "scene.hpp"
#include "serialization.hpp"

#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <memory>
#include <set>

namespace
{

using namespace dc;

glm::mat4 to_glm(const aiMatrix4x4 &matrix)
{
  return glm::transpose(glm::make_mat4(&matrix.a1));
}

struct SceneImportData
{
  std::filesystem::path base_path_;
  std::string           scene_name_;

  std::shared_ptr<Scene>   scene_;

  std::set<std::string>                                materials_;
  std::unordered_map<std::string, TextureDescription>  textures_;
};

std::string load_mesh(const aiScene   *ai_scene,
                      Entity           parent_entity,
                      aiMesh          *ai_mesh,
                      SceneImportData &import_data)
{
  auto entity = import_data.scene_->create_entity(ai_mesh->mName.C_Str());
  parent_entity.add_child(entity);

  MeshImportData mesh_import_data{};
  mesh_import_data.mesh_name_ = import_data.scene_name_;
  mesh_import_data.base_path_ = import_data.base_path_;
  mesh_import_data.materials_ = std::move(import_data.materials_);
  mesh_import_data.textures_  = std::move(import_data.textures_);

  aiMatrix4x4 transform{};
  assert(transform.IsIdentity());
  do_import_mesh(ai_scene, ai_mesh, transform, mesh_import_data);

  import_data.materials_ = std::move(mesh_import_data.materials_);
  import_data.textures_  = std::move(mesh_import_data.textures_);

  const auto mesh_asset_name = sanitize_file_path(
      import_data.scene_name_ + "_" + ai_mesh->mName.C_Str() + ".dcmesh");

  AssetDescription asset_description{};
  mesh_import_data.mesh_.save(Engine::instance()->base_directory() /
                                  std::filesystem::path{"meshes"} /
                                  mesh_asset_name,
                              asset_description);

  auto mesh_asset_handle_name =
      std::filesystem::path{"meshes"} / mesh_asset_name;

  auto mesh_asset_handle = std::dynamic_pointer_cast<MeshAssetHandle>(
      Engine::instance()->asset_cache()->load_asset(
          Asset{mesh_asset_handle_name.string()}));
  entity.add_component<ModelComponent>(mesh_asset_handle);

  return mesh_asset_handle_name.string();
}

void import_node(const aiScene        *ai_scene,
                 aiNode               *ai_node,
                 std::optional<Entity> parent_entity,
                 SceneImportData      &import_data)
{
  auto entity = import_data.scene_->create_entity(ai_node->mName.C_Str());
  if (parent_entity.has_value())
  {
    parent_entity.value().add_child(entity);
  }
  entity.set_local_transform_matrix(to_glm(ai_node->mTransformation));

  for (unsigned i = 0; i < ai_node->mNumMeshes; ++i)
  {
    load_mesh(ai_scene,
              entity,
              ai_scene->mMeshes[ai_node->mMeshes[i]],
              import_data);
  }

  for (unsigned i = 0; i < ai_node->mNumChildren; ++i)
  {
    import_node(ai_scene, ai_node->mChildren[i], entity, import_data);
  }
}

void import_scene(const aiScene *ai_scene, SceneImportData &import_data)
{
  import_node(ai_scene, ai_scene->mRootNode, {}, import_data);

  // save imported data
  const auto scene_file_path = sanitize_file_path(
      std::filesystem::path{"scenes"} / (import_data.scene_name_ + ".dcscn"));
  AssetDescription asset_description{};
  import_data.scene_->save(Engine::instance()->base_directory() /
                               scene_file_path,
                           asset_description);
}
} // namespace

namespace dc
{

void import_scene_asset(const std::filesystem::path &file_path,
                        const std::string           &name)
{
  DC_LOG_DEBUG("Import scene from file {}", file_path.string());

  Assimp::Importer importer;
  const auto       ai_scene = importer.ReadFile(
      file_path.string().c_str(),
      aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
          aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights |
          aiProcess_ImproveCacheLocality | aiProcess_GenUVCoords |
          aiProcess_RemoveRedundantMaterials | aiProcess_FindDegenerates |
          aiProcess_FindInvalidData | aiProcess_FindInstances |
          aiProcess_OptimizeMeshes | aiProcess_CalcTangentSpace);

  if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !ai_scene->mRootNode)
  {
    throw std::runtime_error(std::string("Assimp could not load model: ") +
                             importer.GetErrorString());
  }

  // create neeeded directories
  // TODO: Don't hardcode paths
  const auto base_directory = Engine::instance()->base_directory();
  std::filesystem::create_directories(base_directory / "scenes");
  std::filesystem::create_directories(base_directory / "materials");
  std::filesystem::create_directories(base_directory / "textures");
  std::filesystem::create_directories(base_directory / "meshes");

  SceneImportData import_data{};
  import_data.scene_name_ = name;
  import_data.scene_     = Scene::create();
  import_data.base_path_ = file_path.parent_path();
  import_scene(ai_scene, import_data);
}

} // namespace dc
