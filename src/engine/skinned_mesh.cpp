#include "skinned_mesh.hpp"
#include "skeleton.hpp"

namespace dc
{

SkinnedSubMesh::SkinnedSubMesh(std::unique_ptr<GlVertexArray> vertex_array,
                               std::shared_ptr<MaterialAssetHandle> material)
    : vertex_array_{std::move(vertex_array)},
      material_{material}
{
}

SkinnedSubMesh::SkinnedSubMesh(SkinnedSubMesh &&other)
    : vertex_array_{std::move(other.vertex_array_)},
      material_{std::move(other.material_)}
{
  other.vertex_array_ = nullptr;
  other.material_     = nullptr;
}

void SkinnedSubMesh::operator=(SkinnedSubMesh &&other)
{
  vertex_array_       = std::move(other.vertex_array_);
  other.vertex_array_ = nullptr;
  material_           = std::move(other.material_);
  other.material_     = nullptr;
}

GlVertexArray *SkinnedSubMesh::vertex_array() const
{
  return vertex_array_.get();
}

Material *SkinnedSubMesh::material() const
{

  if (!material_->is_ready())
  {
    return nullptr;
  }

  return material_->get().get();
}

SkinnedMesh::SkinnedMesh(
    std::shared_ptr<Skeleton>                    skeleton,
    std::vector<std::unique_ptr<SkinnedSubMesh>> sub_meshes)
    : skeleton_{skeleton},
      sub_meshes_{std::move(sub_meshes)}
{
}

std::shared_ptr<Skeleton> SkinnedMesh::skeleton() const { return skeleton_; }

std::vector<SkinnedSubMesh *> SkinnedMesh::sub_meshes() const
{
  std::vector<SkinnedSubMesh *> raw_sub_meshes(sub_meshes_.size());
  for (std::size_t i = 0; i < raw_sub_meshes.size(); ++i)
  {
    raw_sub_meshes[i] = sub_meshes_[i].get();
  }
  return raw_sub_meshes;
}

} // namespace dc
