#pragma once

#include "gl_vertex_array.hpp"
#include "material.hpp"
#include "material_asset.hpp"

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
  std::unique_ptr<GlVertexArray> vertex_array_;
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

private:
  std::vector<std::unique_ptr<SubMesh>> meshes_;
};

} // namespace dc
