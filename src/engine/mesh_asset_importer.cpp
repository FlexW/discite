#include "mesh_asset_importer.hpp"
#include "defer.hpp"
#include "engine.hpp"
#include "image.hpp"
#include "importer.hpp"
#include "log.hpp"
#include "material.hpp"
#include "math.hpp"
#include "mesh.hpp"
#include "serialization.hpp"

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
glm::vec3 to_glm(const aiVector3t<float> &value)
{
  return {value.x, value.y, value.z};
}
} // namespace

std::string import_texture(aiMaterial     *ai_material,
                           aiTextureType   ai_texture_type,
                           MeshImportData &import_data)
{
  const auto texture_count = ai_material->GetTextureCount(ai_texture_type);
  if (texture_count == 0)
  {
    return {};
  }
  else if (texture_count > 1)
  {
    LOG_WARN()
        << "Mesh has more than one texture defined. Can just handle one.";
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

  const auto imported_file_path = sanitize_file_path(
      std::filesystem::path{"textures"} /
      (import_data.mesh_name_ + "_" + file_path.stem().string() + ".dctex"));

  if (import_data.textures_.find(imported_file_path) !=
      import_data.textures_.end())
  {
    // texture was already imported
    return imported_file_path;
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
    asset_description.original_file_ = normalize_path(file_path);
    texture_description.save(Engine::instance()->base_directory() /
                                 imported_file_path,
                             asset_description);

    import_data.textures_[imported_file_path] = std::move(texture_description);
  }
  catch (const std::runtime_error &error)
  {
    LOG_WARN() << "Could not import texture: " << error.what();
  }

  return imported_file_path;
}

std::filesystem::path numerate_file_path(const std::filesystem::path &path,
                                         int                          number)
{
  return path.parent_path() /
         (path.stem().string() + "_" + std::to_string(number) +
          path.extension().string());
}

std::string import_material(const aiScene  *ai_scene,
                            const aiMesh   *ai_mesh,
                            MeshImportData &import_data)
{
  const auto ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

  const auto        base_directory = Engine::instance()->base_directory();
  const std::string material_name{ai_material->GetName().C_Str()};

  const auto original_material_import_file_path = sanitize_file_path(
      std::filesystem::path{"materials"} /
      (import_data.mesh_name_ + "_" + material_name + ".dcmat"));
  auto material_import_file_path = original_material_import_file_path;

  // find a unique_material name
  int i{1};
  while (import_data.materials_.find(material_import_file_path) !=
         import_data.materials_.end())
  {
    material_import_file_path =
        numerate_file_path(original_material_import_file_path, i);
    ++i;
  }
  import_data.materials_.insert(material_import_file_path);

  LOG_DEBUG() << "Import material " << material_import_file_path;

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

  return material_import_file_path;
}

void do_import_mesh(const aiScene  *ai_scene,
                    const aiMesh   *ai_mesh,
                    aiMatrix4x4    &transform,
                    MeshImportData &import_data)
{
  const std::string mesh_name = ai_mesh->mName.C_Str();

  // Load vertices
  std::vector<Vertex> vertices;
  vertices.reserve(ai_mesh->mNumVertices);

  for (unsigned j = 0; j < ai_mesh->mNumVertices; ++j)
  {
    Vertex vertex{};

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
      LOG_WARN()
          << "Vertex has more than one texture coordinate. Only one texture "
             "coordinate per vertex will be extracted.";
    }

    vertices.push_back(vertex);
  }

  // Load indices
  LOG_DEBUG() << "Load " << ai_mesh->mNumFaces << " faces in mesh "
              << ai_mesh->mName.C_Str();

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

  auto vertex_array  = std::make_unique<GlVertexArray>();
  auto vertex_buffer = std::make_shared<GlVertexBuffer>();
  auto index_buffer  = std::make_shared<GlIndexBuffer>();

  GlVertexBufferLayout layout;
  layout.push_float(3); // position
  layout.push_float(3); // normal
  layout.push_float(3); // tangent
  layout.push_float(3); // bitanget
  layout.push_float(2); // tex coords
  vertex_buffer->set_data(vertices, layout);

  index_buffer->set_data(indices);

  vertex_array->add_vertex_buffer(vertex_buffer);
  vertex_array->set_index_buffer(index_buffer);

  SubMeshDescription sub_mesh_description{};
  sub_mesh_description.indices_       = std::move(indices);
  sub_mesh_description.vertices_      = std::move(vertices);
  sub_mesh_description.material_name_ = std::move(material);
  import_data.mesh_.sub_meshes_.emplace_back(std::move(sub_mesh_description));
}

void do_import_meshes(const aiScene  *ai_scene,
                      aiNode         *ai_node,
                      aiMatrix4x4    &parent_transform,
                      MeshImportData &import_data)
{
  auto transform = parent_transform * ai_node->mTransformation;

  for (unsigned i = 0; i < ai_node->mNumMeshes; ++i)
  {
    auto              ai_mesh   = ai_scene->mMeshes[ai_node->mMeshes[i]];
    const std::string mesh_name = ai_mesh->mName.C_Str();
    LOG_DEBUG() << "Found mesh " << mesh_name << " in node "
                << ai_node->mName.C_Str();

    do_import_mesh(ai_scene, ai_mesh, transform, import_data);
  }

  for (unsigned i = 0; i < ai_node->mNumChildren; ++i)
  {
    do_import_meshes(ai_scene, ai_node->mChildren[i], transform, import_data);
  }
}

void import_mesh(const aiScene *ai_scene, MeshImportData &import_data)
{
  aiMatrix4x4 transform{};
  assert(transform.IsIdentity());

  do_import_meshes(ai_scene, ai_scene->mRootNode, transform, import_data);

  // save imported data
  AssetDescription asset_description{};
  const auto       mesh_file_path = sanitize_file_path(
      std::filesystem::path{"meshes"} / (import_data.mesh_name_ + ".dcmesh"));
  import_data.mesh_.save(Engine::instance()->base_directory() / mesh_file_path,
                         asset_description);
}

void import_mesh_asset(const std::filesystem::path &file_path,
                       const std::string           &name)
{
  LOG_DEBUG() << "Import mesh from file " << file_path.string().c_str();

  Assimp::Importer importer;
  const auto       ai_scene = importer.ReadFile(
      file_path.string().c_str(),
      aiProcess_JoinIdenticalVertices | aiProcess_Triangulate |
          aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices |
          aiProcess_LimitBoneWeights | aiProcess_ImproveCacheLocality |
          aiProcess_GenUVCoords | aiProcess_RemoveRedundantMaterials |
          aiProcess_FindDegenerates | aiProcess_FindInvalidData |
          aiProcess_FindInstances | aiProcess_OptimizeMeshes |
          aiProcess_CalcTangentSpace);

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

  MeshImportData import_data{};
  import_data.mesh_name_ = name;
  import_data.base_path_ = file_path.parent_path();
  import_mesh(ai_scene, import_data);
}
