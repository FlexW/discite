#include "skinned_mesh.hpp"

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
    Skeleton                                     skeleton,
    std::vector<std::unique_ptr<SkinnedSubMesh>> sub_meshes)
    : skeleton_{std::move(skeleton)},
      sub_meshes_{std::move(sub_meshes)}
{
}

SkinnedMesh::SkinnedMesh(SkinnedMesh &&other) noexcept
{
  skeleton_   = std::move(other.skeleton_);
  sub_meshes_ = std::move(other.sub_meshes_);
}

void SkinnedMesh::operator=(SkinnedMesh &&other) noexcept
{

  skeleton_   = std::move(other.skeleton_);
  sub_meshes_ = std::move(other.sub_meshes_);
}

std::vector<SkinnedSubMesh *> SkinnedMesh::sub_meshes() const
{
  std::vector<SkinnedSubMesh *> raw_sub_meshes(sub_meshes_.size());
  for (std::size_t i = 0; i < raw_sub_meshes.size(); ++i)
  {
    raw_sub_meshes[i] = sub_meshes_[i].get();
  }
  return raw_sub_meshes;
}
void SkinnedMesh::play_animation(const std::string &name)
{
  skeleton_.play_animation(name);
}

void SkinnedMesh::stop_current_animation()
{
  skeleton_.stop_current_animation();
}

void SkinnedMesh::compute_bone_transforms(float delta_time)
{
  return skeleton_.compute_bone_transforms(delta_time);
}

std::vector<glm::mat4> SkinnedMesh::bone_transforms() const
{
  return skeleton_.bone_transforms();
}

std::string SkinnedMesh::current_animation_name() const
{
  return skeleton_.current_animation_name();
}

void SkinnedMesh::set_animation_endless(bool value)
{
  skeleton_.set_animation_endless(value);
}

bool SkinnedMesh::is_animation_endless() const
{
  return skeleton_.is_animation_endless();
}

} // namespace dc
