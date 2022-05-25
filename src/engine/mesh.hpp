#pragma once

#include "asset_description.hpp"
#include "material.hpp"
#include "material_asset.hpp"
#include "platform/gl/gl_vertex_array.hpp"

#include <filesystem>
#include <memory>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace dc
{

struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 tex_coords;
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

class SubMesh
{
public:
  SubMesh(std::unique_ptr<GlVertexArray>       vertex_array,
          std::shared_ptr<MaterialAssetHandle> material);

  SubMesh(SubMesh &&other);
  void operator=(SubMesh &&other);

  GlVertexArray *vertex_array() const;
  Material      *material() const;

private:
  std::unique_ptr<GlVertexArray>       vertex_array_;
  std::shared_ptr<MaterialAssetHandle> material_;

  SubMesh(const SubMesh &) = delete;
  void operator=(const SubMesh &) = delete;
};

class Mesh
{
public:
  Mesh() = default;
  Mesh(Mesh &&other);
  void operator=(Mesh &&other);

  void set_meshes(std::vector<std::unique_ptr<SubMesh>> meshes);
  std::vector<SubMesh *> meshes() const;

  void            set_description(MeshDescription value);
  MeshDescription get_description() const;

private:
  MeshDescription mesh_description_;

  std::vector<std::unique_ptr<SubMesh>> meshes_;
};

} // namespace dc
