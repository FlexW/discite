#include "mesh.hpp"
#include "gl.hpp"
#include "gl_index_buffer.hpp"
#include "gl_texture.hpp"
#include "gl_vertex_buffer.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "log.hpp"
#include "math.hpp"
#include "texture_cache.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stdexcept>

namespace
{

struct Vertex
{
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 tangent;
  glm::vec3 bitangent;
  glm::vec2 tex_coords;
};

glm::vec3 to_glm(const aiVector3t<float> &value)
{
  return {value.x, value.y, value.z};
}

std::shared_ptr<GlTexture> import_texture(aiMaterial   *ai_material,
                                          aiTextureType ai_texture_type,
                                          TextureCache &texture_cache)
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

  try
  {
    return texture_cache.get(path.C_Str());
  }
  catch (const std::runtime_error &error)
  {
    LOG_WARN() << "Could not load texture: " << error.what();
  }

  return nullptr;
}

std::unique_ptr<Material> import_material(const aiScene *ai_scene,
                                          const aiMesh  *ai_mesh,
                                          TextureCache  &texture_cache)
{
  const auto ai_material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

  auto diffuse_texture =
      import_texture(ai_material, aiTextureType_DIFFUSE, texture_cache);
  if (!diffuse_texture)
  {
    diffuse_texture =
        import_texture(ai_material, aiTextureType_BASE_COLOR, texture_cache);
  }
  const auto normal_texture =
      import_texture(ai_material, aiTextureType_NORMALS, texture_cache);

  const std::string material_name(ai_material->GetName().C_Str());
  auto              material = std::make_unique<Material>();

  material->set_diffuse_texture(diffuse_texture);
  material->set_normal_texture(normal_texture);

  return material;
}

void do_load_model(const aiScene                      *ai_scene,
                   aiNode                             *ai_node,
                   aiMatrix4x4                        &parent_transform,
                   std::vector<std::unique_ptr<Mesh>> &meshes,
                   TextureCache                       &texture_cache)
{
  auto generated_dummy_texture_coords = false;

  auto transform = parent_transform * ai_node->mTransformation;

  for (unsigned i = 0; i < ai_node->mNumMeshes; ++i)
  {
    auto              ai_mesh   = ai_scene->mMeshes[ai_node->mMeshes[i]];
    const std::string mesh_name = ai_mesh->mName.C_Str();

    LOG_DEBUG() << "Found mesh " << mesh_name << " in node "
                << ai_node->mName.C_Str();

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
      else
      {
        generated_dummy_texture_coords = true;
        // Generate dummy texture coords anyway
        vertex.tex_coords = glm::vec2{0.0f, 0.0f};
      }

      if (ai_mesh->HasTextureCoords(1))
      {
        LOG_WARN()
            << "Vertex has more than one texture coordinate. Only one texture "
               "coordinate per vertex will be extracted.";
      }

      vertices.push_back(vertex);
    }

    if (generated_dummy_texture_coords)
    {
      LOG_WARN()
          << "Model contained no texture coordinates. Generated dummy texture "
             "coordinates. Textures will look off on this model";
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

    auto material = import_material(ai_scene, ai_mesh, texture_cache);

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

    auto mesh =
        std::make_unique<Mesh>(std::move(vertex_array), std::move(material));
    meshes.push_back(std::move(mesh));
  }

  for (unsigned i = 0; i < ai_node->mNumChildren; ++i)
  {
    do_load_model(ai_scene,
                  ai_node->mChildren[i],
                  transform,
                  meshes,
                  texture_cache);
  }
}

std::vector<std::unique_ptr<Mesh>> load_model(const aiScene *ai_scene,
                                              TextureCache  &texture_cache)
{
  aiMatrix4x4 transform;
  assert(transform.IsIdentity());

  std::vector<std::unique_ptr<Mesh>> meshes;
  do_load_model(ai_scene,
                ai_scene->mRootNode,
                transform,
                meshes,
                texture_cache);
  return meshes;
}

} // namespace

Mesh::Mesh(std::unique_ptr<GlVertexArray> vertex_array,
           std::unique_ptr<Material>      material)
    : vertex_array_{std::move(vertex_array)},
      material_{std::move(material)}
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

Material *Mesh::material() const { return material_.get(); }

Model::Model() { recalculate_model_matrix(); }

Model::Model(Model &&other) { meshes_ = std::move(other.meshes_); }

void Model::operator=(Model &&other) { meshes_ = std::move(other.meshes_); }

void Model::load_from_file(const std::filesystem::path &file_path,
                           TextureCache                &texture_cache)
{
  LOG_DEBUG() << "Import mesh from file " << file_path.string().c_str();

  Assimp::Importer importer;
  const auto       ai_scene =
      importer.ReadFile(file_path.string().c_str(),
                        aiProcess_Triangulate |
                            /*aiProcess_FlipUVs |*/ aiProcess_GenNormals |
                            aiProcess_CalcTangentSpace);

  if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !ai_scene->mRootNode)
  {
    throw std::runtime_error(std::string("Assimp could not load model: ") +
                             importer.GetErrorString());
  }

  meshes_ = load_model(ai_scene, texture_cache);
}

std::vector<Mesh *> Model::meshes() const
{

  std::vector<Mesh *> raw_meshes(meshes_.size());
  for (std::size_t i = 0; i < raw_meshes.size(); ++i)
  {
    raw_meshes[i] = meshes_[i].get();
  }
  return raw_meshes;
}

void Model::set_position(const glm::vec3 &value)
{
  position_ = value;
  recalculate_model_matrix();
}

glm::vec3 Model::position() const { return position_; }

void Model::set_rotation(const glm::quat &value)
{
  rotation_ = value;
  recalculate_model_matrix();
}

void Model::set_scale(const glm::vec3 &value)
{
  scale_ = value;
  recalculate_model_matrix();
}

glm::vec3 Model::scale() const { return scale_; }

glm::mat4 Model::model_matrix() const { return model_matrix_; }

void Model::recalculate_model_matrix()
{
  glm::mat4 model_matrix{1.0f};

  model_matrix = glm::translate(model_matrix, position_);
  model_matrix_ *= glm::toMat4(rotation_);
  model_matrix = glm::scale(model_matrix, scale_);

  model_matrix_ = model_matrix;
}
