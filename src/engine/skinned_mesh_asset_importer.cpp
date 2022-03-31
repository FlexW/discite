#include "skinned_mesh_asset_importer.hpp"
#include "assimp/vector3.h"
#include "defer.hpp"
#include "engine.hpp"
#include "image.hpp"
#include "importer.hpp"
#include "log.hpp"
#include "material.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "serialization.hpp"
#include "skinned_mesh.hpp"

#include <assimp/GltfMaterial.h>
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>
#include <stb_image_write.h>

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace
{
using namespace dc;

glm::vec3 to_glm(const aiVector3D &value)
{
  return {value.x, value.y, value.z};
}

glm::mat4 to_glm(const aiMatrix4x4 &m)
{
  return glm::transpose(glm::make_mat4(&m.a1));
}

glm::quat to_glm(const aiQuaternion &q)
{
  return glm::quat(q.w, q.x, q.y, q.z);
}

void do_create_ai_node_necessary_map(
    aiNode                             *ai_node,
    std::unordered_map<aiNode *, bool> &ai_nodes_map)
{
  ai_nodes_map[ai_node] = false;
  DC_LOG_DEBUG("Found ai node: {}", ai_node->mName.C_Str());

  for (uint32_t i = 0; i < ai_node->mNumChildren; ++i)
  {
    do_create_ai_node_necessary_map(ai_node->mChildren[i], ai_nodes_map);
  }
}

std::unordered_map<aiNode *, bool>
create_ai_node_necessary_map(const aiScene *const ai_scene)
{
  std::unordered_map<aiNode *, bool> ai_nodes_map;

  do_create_ai_node_necessary_map(ai_scene->mRootNode, ai_nodes_map);

  return ai_nodes_map;
}

aiNode *do_find_ai_node_by_name(aiNode *ai_node, const std::string &name)
{
  if (std::string(ai_node->mName.C_Str()) == name)
  {
    return ai_node;
  }

  for (uint32_t i = 0; i < ai_node->mNumChildren; ++i)
  {
    auto ai_found_node = do_find_ai_node_by_name(ai_node->mChildren[i], name);

    if (ai_found_node)
    {
      return ai_found_node;
    }
  }

  return nullptr;
}

aiNode *find_ai_node_by_name(const aiScene     *ai_scene,
                             const std::string &node_name)
{
  return do_find_ai_node_by_name(ai_scene->mRootNode, node_name);
}

void do_mark_parent_ai_nodes_necessary(
    aiNode                             *ai_node,
    aiNode                             *ai_mesh_node,
    aiNode                             *ai_mesh_parent_node,
    std::unordered_map<aiNode *, bool> &ai_nodes_map)
{
  if (ai_node == ai_mesh_node || ai_node == ai_mesh_parent_node)
  {
    return;
  }

  ai_nodes_map[ai_node] = true;
  DC_LOG_DEBUG("Found non bone node that is necessary for skeleton: {}",
               ai_node->mName.C_Str());

  do_mark_parent_ai_nodes_necessary(ai_node->mParent,
                                    ai_mesh_node,
                                    ai_mesh_parent_node,
                                    ai_nodes_map);
}

void mark_parent_ai_nodes_necessary(
    aiNode                             *ai_bone_node,
    aiNode                             *ai_mesh_node,
    std::unordered_map<aiNode *, bool> &ai_nodes_map)
{
  auto ai_mesh_parent_node = ai_mesh_node->mParent;
  do_mark_parent_ai_nodes_necessary(ai_bone_node->mParent,
                                    ai_mesh_node,
                                    ai_mesh_parent_node,
                                    ai_nodes_map);
}

aiNode *do_find_ai_node_from_ai_mesh_by_name(const aiScene     *ai_scene,
                                             aiNode            *ai_node,
                                             const std::string &mesh_name)
{
  for (uint32_t i = 0; i < ai_node->mNumMeshes; ++i)
  {
    auto ai_mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
    if (std::string(ai_mesh->mName.C_Str()) == mesh_name)
    {
      return ai_node;
    }
  }

  for (uint32_t i = 0; i < ai_node->mNumChildren; ++i)
  {
    auto ai_node_found =
        do_find_ai_node_from_ai_mesh_by_name(ai_scene,
                                             ai_node->mChildren[i],
                                             mesh_name);

    if (ai_node_found)
    {
      return ai_node_found;
    }
  }

  return nullptr;
}

aiNode *find_ai_node_from_ai_mesh_by_name(const aiScene     *ai_scene,
                                          const std::string &mesh_name)
{
  return do_find_ai_node_from_ai_mesh_by_name(ai_scene,
                                              ai_scene->mRootNode,
                                              mesh_name);
}

void find_necessary_ai_nodes(const aiScene                      *ai_scene,
                             std::unordered_map<aiNode *, bool> &ai_nodes_map)
{
  auto ai_meshes = ai_scene->mMeshes;
  for (uint32_t i = 0; i < ai_scene->mNumMeshes; ++i)
  {
    auto ai_mesh = ai_meshes[i];

    for (uint32_t j = 0; j < ai_mesh->mNumBones; ++j)
    {
      auto        ai_bone      = ai_mesh->mBones[j];
      std::string bone_name    = ai_bone->mName.C_Str();
      auto        ai_bone_node = find_ai_node_by_name(ai_scene, bone_name);

      if (!ai_bone_node)
      {
        DC_FAIL("Can not find node to bone");
      }

      DC_LOG_DEBUG("Found necessary ai node: {}", bone_name);
      ai_nodes_map[ai_bone_node] = true;

      auto ai_mesh_node =
          find_ai_node_from_ai_mesh_by_name(ai_scene, ai_mesh->mName.C_Str());
      if (!ai_mesh_node)
      {
        DC_LOG_DEBUG("Could not find ai node for ai mesh {}",
                     ai_mesh->mName.C_Str());
      }
      else
      {
        mark_parent_ai_nodes_necessary(ai_bone_node,
                                       ai_mesh_node,
                                       ai_nodes_map);
      }
    }
  }
}

aiMatrix4x4 do_calculate_skeleton_transform(aiNode *ai_node)
{
  if (ai_node->mParent == nullptr)
  {
    return ai_node->mTransformation;
  }

  return do_calculate_skeleton_transform(ai_node->mParent) *
         ai_node->mTransformation;
}

aiMatrix4x4 calculate_skeleton_transform(const aiScene     *ai_scene,
                                         const std::string &name)
{
  aiNode *ai_node = find_ai_node_by_name(ai_scene, name);
  DC_ASSERT(ai_node, "Node not set");

  if (ai_node->mParent == nullptr)
  {
    return ai_node->mTransformation;
  }

  return do_calculate_skeleton_transform(ai_node->mParent);
}

void do_build_bones(aiNode                             *ai_node,
                    std::unordered_map<aiNode *, bool> &ai_nodes_map,
                    int                                 parent_index,
                    std::vector<Bone>                  &bones)
{
  auto iter = ai_nodes_map.find(ai_node);
  if (iter == ai_nodes_map.end())
  {
    DC_FAIL("Ai node is not in ai nodes map");
  }

  auto bone_index = parent_index;

  auto necessary = iter->second;
  if (necessary)
  {
    Bone bone;
    bone.name_            = ai_node->mName.C_Str();
    bone.parent_index_    = parent_index;
    bone.local_bind_pose_ = to_glm(ai_node->mTransformation);

    bones.push_back(bone);
    DC_LOG_DEBUG("Add node to skeleton: {}", bone.name_);

    bone_index = bones.size() - 1;
  }
  else
  {
    DC_LOG_DEBUG("Node {} not necessary for skeleton", ai_node->mName.C_Str());
  }

  for (uint32_t i = 0; i < ai_node->mNumChildren; ++i)
  {
    do_build_bones(ai_node->mChildren[i], ai_nodes_map, bone_index, bones);
  }
}

std::vector<Bone> build_bones(const aiScene                      *ai_scene,
                              std::unordered_map<aiNode *, bool> &ai_nodes_map)
{
  std::vector<Bone> bones;

  do_build_bones(ai_scene->mRootNode, ai_nodes_map, -1, bones);
  DC_ASSERT(bones.size() > 0, "Bones not build");

  auto skeleton_transform =
      to_glm(calculate_skeleton_transform(ai_scene, bones[0].name_));

  bones[0].local_bind_pose_ = skeleton_transform * bones[0].local_bind_pose_;

  return bones;
}

std::vector<Animation> load_animations(const aiScene *ai_scene,
                                       Skeleton &     skeleton)
{
  std::vector<Animation> animations;

  // Traverse all animations and load animation tracks. A track is a sequence of
  // keyframes that animate a bone/node
  for (uint32_t i = 0; i < ai_scene->mNumAnimations; ++i)
  {
    std::vector<std::optional<BoneTransform>> tracks;
    tracks.resize(skeleton.bones_count());

    // Traverse all tracks
    const auto  ai_animation = ai_scene->mAnimations[i];
    std::string animation_name{ai_animation->mName.C_Str()};
    if (animation_name.empty())
    {
      animation_name = fmt::format("Animation {}", i);
      DC_LOG_DEBUG("Animation has no name. Give it name {}", animation_name);
    }
    DC_LOG_DEBUG("Found animation: {}", animation_name);
    for (uint32_t j = 0; j < ai_animation->mNumChannels; ++j)
    {
      const auto ai_node_anim = ai_animation->mChannels[j];
      const auto bone_index =
          skeleton.bone_index(ai_node_anim->mNodeName.C_Str());
      if (bone_index == -1)
      {
        DC_FAIL("Skeleton contains no bone name: {}",
                ai_node_anim->mNodeName.C_Str());
      }

      DC_LOG_DEBUG("Animation {} has track {}",
                   animation_name,
                   ai_node_anim->mNodeName.C_Str());

      BoneTransform bone_transform;

      // Load position keys
      for (uint32_t k = 0; k < ai_node_anim->mNumPositionKeys; ++k)
      {
        const auto ai_vector_key = ai_node_anim->mPositionKeys[k];

        BoneTranslation bone_translation;
        bone_translation.position_ = to_glm(ai_vector_key.mValue);
        bone_translation.time_     = ai_vector_key.mTime;

        bone_transform.bone_translation_.emplace_back(bone_translation);
      }

      // Load scale keys
      for (uint32_t k = 0; k < ai_node_anim->mNumScalingKeys; ++k)
      {
        const auto ai_vector_key = ai_node_anim->mScalingKeys[k];

        BoneScaling bone_scaling;
        bone_scaling.scale_ = to_glm(ai_vector_key.mValue);
        bone_scaling.time_  = ai_vector_key.mTime;

        bone_transform.bone_scaling_.emplace_back(bone_scaling);
      }

      // Load rotation keys
      for (uint32_t k = 0; k < ai_node_anim->mNumRotationKeys; ++k)
      {
        const auto ai_quat_key = ai_node_anim->mRotationKeys[k];

        BoneRotation bone_rotation;
        bone_rotation.rotation_ = to_glm(ai_quat_key.mValue);
        bone_rotation.time_     = ai_quat_key.mTime;

        bone_transform.bone_rotation_.emplace_back(bone_rotation);
      }

      tracks[bone_index] = bone_transform;
    }

    const auto  duration         = ai_animation->mDuration;
    const auto  ticks_per_second = ai_animation->mTicksPerSecond;

    Animation animation(animation_name, duration, ticks_per_second, tracks);

    animations.emplace_back(animation);
  }

  return animations;
}

void compute_global_inv_bind_poses(std::vector<Bone> &bones)
{
  if (bones.size() == 0)
  {
    return;
  }

  // First compute global bind pose

  // Root bone is special
  bones[0].global_inv_bind_pose_ = bones[0].local_bind_pose_;
  for (uint32_t i = 1; i < bones.size(); ++i)
  {
    // Bones are ordered, parent bone comes always before child bone
    bones[i].global_inv_bind_pose_ =
        bones[bones[i].parent_index_].global_inv_bind_pose_ *
        bones[i].local_bind_pose_;
  }

  // Then invert everything
  for (auto &bone : bones)
  {
    bone.global_inv_bind_pose_ = glm::inverse(bone.global_inv_bind_pose_);
  }
}

std::string import_texture(aiMaterial            *ai_material,
                           aiTextureType          ai_texture_type,
                           SkinnedMeshImportData &import_data)
{
  const auto texture_count = ai_material->GetTextureCount(ai_texture_type);
  if (texture_count == 0)
  {
    return {};
  }
  else if (texture_count > 1)
  {
    DC_LOG_WARN("Mesh has more than one texture defined. Can just handle one.");
  }

  aiString path{};
  ai_material->GetTexture(ai_texture_type, 0, &path);

  // replace \ by / if needed
  for (std::size_t i = 0; i < path.length; ++i)
  {
    if (path.data[i] == '\\')
    {
      path.data[i] = '/';
    }
  }

  const std::filesystem::path file_path{import_data.base_path_ / path.C_Str()};

  const auto imported_file_path =
      sanitize_file_path(std::filesystem::path{"textures"} /
                         (import_data.skinned_mesh_name_ + "_" +
                          file_path.stem().string() + ".dctex"));

  if (import_data.textures_.find(imported_file_path.string()) !=
      import_data.textures_.end())
  {
    // texture was already imported
    return imported_file_path.string();
  }

  TextureDescription texture_description{};
  try
  {
    // load image and make sure it gets saved as png
    Image                     image{file_path};
    std::vector<std::uint8_t> png_data;
    stbi_write_png_to_func(
        [](void *context, void *data, int size)
        {
          const auto png_data = static_cast<std::vector<uint8_t> *>(context);
          png_data->resize(size);
          std::memcpy(png_data->data(), data, size);
        },
        &png_data,
        image.width(),
        image.height(),
        image.channels_count(),
        static_cast<const void *>(image.data()),
        0);
    texture_description.data_ = std::move(png_data);

    AssetDescription asset_description{};
    asset_description.original_file_ = normalize_path(file_path).string();
    texture_description.save(Engine::instance()->base_directory() /
                                 imported_file_path,
                             asset_description);

    import_data.textures_[imported_file_path.string()] =
        std::move(texture_description);
  }
  catch (const std::runtime_error &error)
  {
    DC_LOG_WARN("Could not import texture: {}", error.what());
  }

  return imported_file_path.string();
}

std::filesystem::path numerate_file_path(const std::filesystem::path &path,
                                         int                          number)
{
  return path.parent_path() /
         (path.stem().string() + "_" + std::to_string(number) +
          path.extension().string());
}

std::string import_material(const aiScene         *ai_scene,
                            const aiMesh          *ai_mesh,
                            SkinnedMeshImportData &import_data)
{
  const auto ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

  const auto        base_directory = Engine::instance()->base_directory();
  const std::string material_name{ai_material->GetName().C_Str()};

  const auto original_material_import_file_path = sanitize_file_path(
      std::filesystem::path{"materials"} /
      (import_data.skinned_mesh_name_ + "_" + material_name + ".dcmat"));
  auto material_import_file_path = original_material_import_file_path;

  // find a unique_material name
  int i{1};
  while (import_data.materials_.find(material_import_file_path.string()) !=
         import_data.materials_.end())
  {
    material_import_file_path =
        numerate_file_path(original_material_import_file_path, i);
    ++i;
  }
  import_data.materials_.insert(material_import_file_path.string());

  DC_LOG_DEBUG("Import material ", material_import_file_path.string());

  MaterialDescription material_description{};

  material_description.albedo_texture_name_ =
      import_texture(ai_material, aiTextureType_BASE_COLOR, import_data);

  material_description.emissive_texture_name_ =
      import_texture(ai_material, aiTextureType_EMISSIVE, import_data);

  material_description.roughness_texture_name_ =
      import_texture(ai_material, aiTextureType_UNKNOWN, import_data);

  material_description.ambient_occlusion_texture_name_ =
      import_texture(ai_material, aiTextureType_LIGHTMAP, import_data);

  material_description.normal_texture_name_ =
      import_texture(ai_material, aiTextureType_NORMALS, import_data);

  aiColor4D color{};
  if (aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_AMBIENT, &color) ==
      AI_SUCCESS)
  {
    material_description.emissive_color_ = {color.r,
                                            color.g,
                                            color.b,
                                            glm::min(color.a, 1.0f)};
  }
  color = {};
  if (aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_DIFFUSE, &color) ==
      AI_SUCCESS)
  {
    material_description.albedo_color_ = {color.r,
                                          color.g,
                                          color.b,
                                          glm::min(color.a, 1.0f)};
  }
  color = {};
  if (aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_EMISSIVE, &color) ==
      AI_SUCCESS)
  {
    material_description.emissive_color_.r += color.r;
    material_description.emissive_color_.g += color.g;
    material_description.emissive_color_.b += color.b;
    material_description.emissive_color_.a += color.a;
    if (material_description.emissive_color_.a > 1.0f)
    {
      material_description.emissive_color_.a = 1.0f;
    }
  }

  const float opaqueness_threshold{0.05f};
  float       opacity{1.0f};

  if (aiGetMaterialFloat(ai_material, AI_MATKEY_OPACITY, &opacity) ==
      AI_SUCCESS)
  {
    material_description.transparency_factor_ =
        glm::clamp(1.0f - opacity, 0.0f, 1.0f);
    if (material_description.transparency_factor_ >=
        1.0f - opaqueness_threshold)
    {
      material_description.transparency_factor_ = 0.0f;
    }
  }

  if (aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_TRANSPARENT, &color) ==
      AI_SUCCESS)
  {
    const auto opacity = std::max(std::max(color.r, color.g), color.b);
    material_description.transparency_factor_ = glm::clamp(opacity, 0.0f, 1.0f);
    if (material_description.transparency_factor_ >=
        1.0f - opaqueness_threshold)
    {
      material_description.transparency_factor_ = 0.0f;
    }
    material_description.alpha_test_ = 0.5f;
  }

  // float tmp = 1.0f;
  // if (aiGetMaterialFloat(ai_material,
  //                        AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLIC_FACTOR,
  //                        &tmp) == AI_SUCCESS)
  // {
  //   material_description.metallic_factor_ = tmp;
  // }

  // if (aiGetMaterialFloat(ai_material,
  //                        AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_ROUGHNESS_FACTOR,
  //                        &tmp) == AI_SUCCESS)
  // {
  //   material_description.roughness_ = {tmp, tmp, tmp, tmp};
  // }

  AssetDescription asset_description{};
  material_description.save(Engine::instance()->base_directory() /
                                material_import_file_path,
                            asset_description);

  return material_import_file_path.string();
}

} // namespace

namespace dc
{

void do_import_skinned_mesh(const aiScene         *ai_scene,
                            const aiMesh          *ai_mesh,
                            aiMatrix4x4           &transform,
                            SkinnedMeshImportData &import_data)
{
  const std::string mesh_name = ai_mesh->mName.C_Str();

  // Load vertices
  std::vector<SkinnedVertex> vertices;
  vertices.reserve(ai_mesh->mNumVertices);

  for (unsigned j = 0; j < ai_mesh->mNumVertices; ++j)
  {
    SkinnedVertex vertex{};

    const auto ai_position = transform * ai_mesh->mVertices[j];
    vertex.position        = to_glm(ai_position);

    if (ai_mesh->HasNormals())
    {
      const auto ai_normal = transform * ai_mesh->mNormals[j];
      vertex.normal        = to_glm(ai_normal);
    }

    if (ai_mesh->HasTangentsAndBitangents())
    {
      const auto ai_tangent   = transform * ai_mesh->mTangents[j];
      const auto ai_bitangent = transform * ai_mesh->mBitangents[j];

      vertex.tangent   = to_glm(ai_tangent);
      vertex.bitangent = to_glm(ai_bitangent);
    }

    if (ai_mesh->HasTextureCoords(0))
    {
      const auto ai_tex_coords = ai_mesh->mTextureCoords[0][j];
      vertex.tex_coords        = glm::vec2{ai_tex_coords.x, ai_tex_coords.y};
    }

    if (ai_mesh->HasTextureCoords(1))
    {
      DC_LOG_WARN(
          "Vertex has more than one texture coordinate. Only one texture "
          "coordinate per vertex will be extracted.");
    }

    vertices.push_back(vertex);
  }

  // load skinning info

  // stores how many bones already loaded for this vertex
  std::unordered_map<uint32_t, uint32_t> vertex_indices_to_bone_count_map;
  vertex_indices_to_bone_count_map.reserve(vertices.size());

  for (unsigned j = 0; j < ai_mesh->mNumBones; ++j)
  {
    const auto ai_bone    = ai_mesh->mBones[j];
    const auto bone_index =
        import_data.skinned_mesh_.skeleton_.bone_index(ai_bone->mName.C_Str());
    DC_ASSERT(bone_index != -1, "Invalid bone index");

    for (uint32_t k = 0; k < ai_bone->mNumWeights; ++k)
    {
      const auto ai_vertex_weight = ai_bone->mWeights[k];
      auto       iter =
          vertex_indices_to_bone_count_map.find(ai_vertex_weight.mVertexId);

      // Are bones for this vertex already inserted?
      if (iter == vertex_indices_to_bone_count_map.end())
      {
        vertex_indices_to_bone_count_map[ai_vertex_weight.mVertexId] = 0;
        vertices[ai_vertex_weight.mVertexId].skin_bones =
            glm::uvec4(bone_index, 0, 0, 0);
        vertices[ai_vertex_weight.mVertexId].bone_weights =
            glm::vec4(ai_vertex_weight.mWeight, 0.0f, 0.0f, 0.0f);
      }
      else
      {
        const auto bone_count = iter->second + 1;
        // More than four bones?
        if (bone_count >= 4)
        {
          auto &vertex = vertices[ai_vertex_weight.mVertexId];
          // Is weight greater?
          bool insert = false;
          for (uint32_t l = 0; l < 4; ++l)
          {
            if (vertex.bone_weights[l] < ai_vertex_weight.mWeight)
            {
              insert = true;
              break;
            }
          }

          // If is greater
          if (insert)
          {
            // Find smallest
            unsigned smallest{0};
            for (unsigned l = 1; l < 4; ++l)
            {
              if (vertex.bone_weights[l] < vertex.bone_weights[smallest])
              {
                smallest = l;
              }
            }

            // Insert it
            vertex.bone_weights[smallest] = ai_vertex_weight.mWeight;
            vertex.skin_bones[smallest]   = bone_index;
          }

          // Now rebalance
          float sum{0.0f};
          for (unsigned l = 0; l < 4; ++l)
          {
            sum += vertex.bone_weights[l];
          }
          for (unsigned l = 0; l < 4; ++l)
          {
            vertex.bone_weights[l] /= sum;
          }
        }
        else
        {
          // Insert bone if not already four bones
          vertex_indices_to_bone_count_map[ai_vertex_weight.mVertexId] =
              bone_count;
          vertices[ai_vertex_weight.mVertexId].skin_bones[bone_count] =
              bone_index;
          vertices[ai_vertex_weight.mVertexId].bone_weights[bone_count] =
              ai_vertex_weight.mWeight;
        }
      }
    }
  }

  // Load indices
  DC_LOG_DEBUG("Load {} faces in mesh {}",
               ai_mesh->mNumFaces,
               ai_mesh->mName.C_Str());

  std::vector<std::uint32_t> indices;
  indices.reserve(ai_mesh->mNumFaces * 3);
  for (std::uint32_t j = 0; j < ai_mesh->mNumFaces; ++j)
  {
    const auto ai_face = ai_mesh->mFaces[j];
    if (ai_face.mNumIndices != 3)
    {
      throw std::runtime_error(
          "Face has not three indices. Only three allowed");
    }
    indices.push_back(ai_face.mIndices[0]);
    indices.push_back(ai_face.mIndices[1]);
    indices.push_back(ai_face.mIndices[2]);
  }

  auto material = import_material(ai_scene, ai_mesh, import_data);

  auto vertex_array = std::make_unique<GlVertexArray>();

  GlVertexBufferLayout layout;
  layout.push_float(3); // position
  layout.push_float(3); // normal
  layout.push_float(3); // tangent
  layout.push_float(3); // bitanget
  layout.push_int(4);   // bones
  layout.push_float(4); // bone weights
  layout.push_float(2); // tex coords
  auto vertex_buffer = std::make_shared<GlVertexBuffer>(vertices, layout);

  auto index_buffer = std::make_shared<GlIndexBuffer>(indices);

  vertex_array->add_vertex_buffer(vertex_buffer);
  vertex_array->set_index_buffer(index_buffer);

  SkinnedSubMeshDescription skinned_sub_mesh_description{};
  skinned_sub_mesh_description.indices_       = std::move(indices);
  skinned_sub_mesh_description.vertices_      = std::move(vertices);
  skinned_sub_mesh_description.material_name_ = std::move(material);
  import_data.skinned_mesh_.sub_meshes_.emplace_back(
      std::move(skinned_sub_mesh_description));
}

void do_import_skinned_meshes(const aiScene         *ai_scene,
                              aiNode                *ai_node,
                              aiMatrix4x4           &parent_transform,
                              SkinnedMeshImportData &import_data)
{
  auto transform = parent_transform * ai_node->mTransformation;

  for (unsigned i = 0; i < ai_node->mNumMeshes; ++i)
  {
    auto              ai_mesh   = ai_scene->mMeshes[ai_node->mMeshes[i]];
    const std::string mesh_name = ai_mesh->mName.C_Str();
    DC_LOG_DEBUG("Found mesh {} in node {}", mesh_name, ai_node->mName.C_Str());

    do_import_skinned_mesh(ai_scene, ai_mesh, transform, import_data);
  }

  for (unsigned i = 0; i < ai_node->mNumChildren; ++i)
  {
    do_import_skinned_meshes(ai_scene,
                             ai_node->mChildren[i],
                             transform,
                             import_data);
  }
}

Skeleton import_skeleton(const aiScene *ai_scene)
{
  // Build up a map with all nodes that are in the scene
  // contained. This map saves for each node if this node is necessary
  // for the skeleton.
  auto ai_nodes_map = create_ai_node_necessary_map(ai_scene);

  // Find nodes that are necessary for the skeleton
  find_necessary_ai_nodes(ai_scene, ai_nodes_map);

  // Extract bones. First bone is the root bone.
  auto bones = build_bones(ai_scene, ai_nodes_map);

  DC_LOG_DEBUG("Bones hierarchy:");
  for (auto &bone : bones)
  {
    DC_LOG_DEBUG("name: {}, parent_index: {}", bone.name_, bone.parent_index_);
  }

  compute_global_inv_bind_poses(bones);

  return Skeleton{bones};
}

void import_skinned_mesh(const aiScene         *ai_scene,
                         SkinnedMeshImportData &import_data)
{
  aiMatrix4x4 transform{};
  assert(transform.IsIdentity());

  import_data.skinned_mesh_.skeleton_ = import_skeleton(ai_scene);
  auto animations =
      load_animations(ai_scene, import_data.skinned_mesh_.skeleton_);
  for (auto &&animation : animations)
  {
    import_data.skinned_mesh_.skeleton_.add_animation(std::move(animation));
  }

  do_import_skinned_meshes(ai_scene,
                           ai_scene->mRootNode,
                           transform,
                           import_data);

  // save imported data
  AssetDescription asset_description{};
  const auto       mesh_file_path =
      sanitize_file_path(std::filesystem::path{"meshes"} /
                         (import_data.skinned_mesh_name_ + ".dcskinmesh"));
  import_data.skinned_mesh_.save(Engine::instance()->base_directory() /
                                     mesh_file_path,
                                 asset_description);
}

void import_skinned_mesh_asset(const std::filesystem::path &file_path,
                               const std::string           &name)
{
  DC_LOG_DEBUG("Import skinned mesh from file {}", file_path.string());

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
  std::filesystem::create_directories(base_directory / "textures");
  std::filesystem::create_directories(base_directory / "materials");
  std::filesystem::create_directories(base_directory / "meshes");

  SkinnedMeshImportData import_data{};
  import_data.skinned_mesh_name_ = name;
  import_data.base_path_         = file_path.parent_path();
  import_skinned_mesh(ai_scene, import_data);
}

} // namespace dc
