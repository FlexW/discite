#pragma once

#include "gl_vertex_array.hpp"
#include "material.hpp"
#include "material_asset.hpp"
#include "mesh.hpp"
#include "skeleton.hpp"
#include <memory>

namespace dc
{

struct SkinnedVertex
{
  glm::vec3  position;
  glm::vec3  normal;
  glm::vec3  tangent;
  glm::vec3  bitangent;
  glm::ivec4 skin_bones;
  glm::vec4  bone_weights;
  glm::vec2  tex_coords;
};

class SkinnedSubMesh
{
public:
  SkinnedSubMesh(std::unique_ptr<GlVertexArray>       vertex_array,
                 std::shared_ptr<MaterialAssetHandle> material);

  SkinnedSubMesh(SkinnedSubMesh &&other);
  void operator=(SkinnedSubMesh &&other);

  GlVertexArray *vertex_array() const;
  Material      *material() const;

private:
  std::unique_ptr<GlVertexArray>       vertex_array_;
  std::shared_ptr<MaterialAssetHandle> material_;

  SkinnedSubMesh(const SkinnedSubMesh &) = delete;
  void operator=(const SkinnedSubMesh &) = delete;
};

class SkinnedMesh
{
public:
  SkinnedMesh(const Skeleton                              &skeleton,
              std::vector<std::unique_ptr<SkinnedSubMesh>> sub_meshes);

  std::vector<SkinnedSubMesh *> sub_meshes() const;

  void set_animation_endless(bool value);
  bool is_animation_endless() const;

  void play_animation(const std::string &name);
  void stop_current_animation();

  std::string current_animation_name() const;

  void                   compute_bone_transforms(float delta_time);
  std::vector<glm::mat4> bone_transforms() const;

private:
  Skeleton                                     skeleton_;
  std::vector<std::unique_ptr<SkinnedSubMesh>> sub_meshes_;
};

} // namespace dc
