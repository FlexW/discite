#include "scene.hpp"
#include "assimp/matrix4x4.h"
#include "assimp/mesh.h"
#include "entity.hpp"
#include "event.hpp"
#include "log.hpp"
#include "mesh.hpp"
#include "model_component.hpp"
#include "name_component.hpp"
#include "relationship_component.hpp"
#include "render_system.hpp"
#include "texture_cache.hpp"
#include "transform_component.hpp"

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <memory>

namespace
{

glm::mat4 to_glm(const aiMatrix4x4 &matrix)
{
  return glm::transpose(glm::make_mat4(&matrix.a1));
}

} // namespace

EventId SceneLoadedEvent::id = 0x87a8e8c9;

SceneLoadedEvent::SceneLoadedEvent(std::shared_ptr<Scene> scene)
    : Event{id},
      scene_{scene}
{
}

EventId SceneUnloadedEvent::id = 0xa26517c8;

SceneUnloadedEvent::SceneUnloadedEvent(std::shared_ptr<Scene> scene)
    : Event{id},
      scene_{scene}
{
}

std::shared_ptr<Scene> Scene::create()
{
  return std::shared_ptr<Scene>(new Scene);
}

void Scene::load_from_file(const std::filesystem::path &file_path,
                           TextureCache                &texture_cache)
{
  LOG_INFO() << "Import scene from file " << file_path.string().c_str();

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

  load_scene(ai_scene, texture_cache);
}

void Scene::load_mesh(const aiScene *ai_scene,
                      Entity         parent_entity,
                      aiMesh        *ai_mesh,
                      TextureCache  &texture_cache)
{
  auto entity = create_entity(ai_mesh->mName.C_Str());
  parent_entity.add_child(entity);

  // Load vertices
  std::vector<Vertex> vertices;
  vertices.reserve(ai_mesh->mNumVertices);

  for (unsigned j = 0; j < ai_mesh->mNumVertices; ++j)
  {
    Vertex vertex{};

    const auto ai_position = ai_mesh->mVertices[j];
    vertex.position        = to_glm(ai_position);

    if (ai_mesh->HasNormals())
    {
      const auto ai_normal = ai_mesh->mNormals[j];
      vertex.normal        = to_glm(ai_normal);
    }

    if (ai_mesh->HasTangentsAndBitangents())
    {
      const auto ai_tangent   = ai_mesh->mTangents[j];
      const auto ai_bitangent = ai_mesh->mBitangents[j];

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

  // Load indices

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
  auto                               model = std::make_shared<Model>();
  std::vector<std::unique_ptr<Mesh>> meshes;
  meshes.push_back(std::move(mesh));
  model->set_meshes(std::move(meshes));

  entity.add_component<ModelComponent>(model);
}

void Scene::load_node(const aiScene        *ai_scene,
                      aiNode               *ai_node,
                      std::optional<Entity> parent_entity,
                      TextureCache         &texture_cache)
{
  auto entity = create_entity(ai_node->mName.C_Str());
  if (parent_entity.has_value())
  {
    parent_entity.value().add_child(entity);
  }
  entity.set_local_transform_matrix(to_glm(ai_node->mTransformation));

  for (unsigned i = 0; i < ai_node->mNumMeshes; ++i)
  {
    load_mesh(ai_scene,
              entity,
              ai_scene->mMeshes[ai_node->mMeshes[i]],
              texture_cache);
  }

  for (unsigned i = 0; i < ai_node->mNumChildren; ++i)
  {
    load_node(ai_scene, ai_node->mChildren[i], entity, texture_cache);
  }
}

void Scene::load_scene(const aiScene *ai_scene, TextureCache &texture_cache)
{
  load_node(ai_scene, ai_scene->mRootNode, {}, texture_cache);
}

void Scene::init_systems()
{
  systems_.emplace_back(std::make_unique<RenderSystem>(shared_from_this()));
}

void Scene::init()
{
  init_systems();

  for (const auto &system : systems_)
  {
    system->init();
  }
}

void Scene::update(float delta_time)
{
  for (const auto &system : systems_)
  {
    system->update(delta_time);
  }
}

void Scene::render(SceneRenderInfo &scene_render_info,
                   ViewRenderInfo  &view_render_info)
{
  for (const auto &system : systems_)
  {
    system->render(scene_render_info, view_render_info);
  }
}

bool Scene::on_event(const Event &event)
{
  for (const auto &system : systems_)
  {
    system->on_event(event);
  }

  return false;
}

entt::registry &Scene::registry() { return registry_; }

Entity Scene::create_entity(const std::string &name)
{
  Entity     entity{registry_.create(), shared_from_this()};
  const auto entity_name = name.empty() ? "Entity" : name;
  entity.add_component<NameComponent>(name);
  entity.add_component<TransformComponent>();
  entity.add_component<RelationshipComponent>();
  return entity;
}
