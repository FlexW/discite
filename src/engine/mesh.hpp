#pragma once

#include "asset_handle.hpp"
#include "gl_vertex_array.hpp"
#include "material.hpp"
#include "material_asset.hpp"
#include "texture_cache.hpp"

#include <filesystem>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// glm::vec3 to_glm(const aiVector3t<float> &value);

// std::shared_ptr<GlTexture> import_texture(aiMaterial   *ai_material,
//                                           aiTextureType ai_texture_type,
//                                           TextureCache &texture_cache);

// std::shared_ptr<Material> import_material(const aiScene *ai_scene,
//                                           const aiMesh  *ai_mesh,
//                                           TextureCache  &texture_cache);

struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 tex_coords;
};

class Mesh
{
public:
  Mesh(std::unique_ptr<GlVertexArray>       vertex_array,
       std::shared_ptr<MaterialAssetHandle> material);

  Mesh(Mesh &&other);
  void operator=(Mesh &&other);

  GlVertexArray *vertex_array() const;
  Material      *material() const;

private:
  std::unique_ptr<GlVertexArray> vertex_array_;
  std::shared_ptr<MaterialAssetHandle> material_;

  Mesh(const Mesh &) = delete;
  void operator=(const Mesh &) = delete;
};

class Model
{
public:
  Model() = default;
  Model(Model &&other);
  void operator=(Model &&other);

  // void load_from_file(const std::filesystem::path &file_path,
  //                     TextureCache                &texture_cache);

  void                set_meshes(std::vector<std::unique_ptr<Mesh>> meshes);
  std::vector<Mesh *> meshes() const;

private:
  std::vector<std::unique_ptr<Mesh>> meshes_;
};
