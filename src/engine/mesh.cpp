#include "mesh.hpp"
#include "gl.hpp"
#include "gl_index_buffer.hpp"
#include "gl_texture.hpp"
#include "gl_vertex_buffer.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "log.hpp"
#include "material_asset.hpp"
#include "math.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>

#include <cstddef>
#include <memory>
#include <stdexcept>

namespace dc
{

Mesh::Mesh(std::unique_ptr<GlVertexArray>       vertex_array,
           std::shared_ptr<MaterialAssetHandle> material)
    : vertex_array_{std::move(vertex_array)},
      material_{material}
{
}

Mesh::Mesh(Mesh &&other)
    : vertex_array_{std::move(other.vertex_array_)},
      material_{std::move(other.material_)}
{
  other.vertex_array_ = nullptr;
  other.material_     = nullptr;
}

void Mesh::operator=(Mesh &&other)
{
  vertex_array_       = std::move(other.vertex_array_);
  other.vertex_array_ = nullptr;
  material_           = std::move(other.material_);
  other.material_     = nullptr;
}

GlVertexArray *Mesh::vertex_array() const { return vertex_array_.get(); }

Material *Mesh::material() const
{

  if (!material_->is_ready())
  {
    return nullptr;
  }

  return material_->get().get();
}

Model::Model(Model &&other) { meshes_ = std::move(other.meshes_); }

void Model::operator=(Model &&other) { meshes_ = std::move(other.meshes_); }

std::vector<Mesh *> Model::meshes() const
{

  std::vector<Mesh *> raw_meshes(meshes_.size());
  for (std::size_t i = 0; i < raw_meshes.size(); ++i)
  {
    raw_meshes[i] = meshes_[i].get();
  }
  return raw_meshes;
}

void Model::set_meshes(std::vector<std::unique_ptr<Mesh>> meshes)
{
  meshes_ = std::move(meshes);
}

} // namespace dc
