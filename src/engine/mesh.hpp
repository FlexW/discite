#pragma once

#include "gl_vertex_array.hpp"
#include "material.hpp"
#include "texture_cache.hpp"

#include <filesystem>
#include <memory>

class Mesh
{
public:
  Mesh(std::unique_ptr<GlVertexArray> vertex_array,
       std::unique_ptr<Material>      material);

  Mesh(Mesh &&other);
  void operator=(Mesh &&other);

  GlVertexArray *vertex_array() const;
  Material      *material() const;

private:
  std::unique_ptr<GlVertexArray> vertex_array_;
  std::unique_ptr<Material>      material_;

  Mesh(const Mesh &) = delete;
  void operator=(const Mesh &) = delete;
};

class Model
{
public:
  Model();
  Model(Model &&other);
  void operator=(Model &&other);

  void load_from_file(const std::filesystem::path &file_path,
                      TextureCache                &texture_cache);

  std::vector<Mesh *> meshes() const;

  void      set_position(const glm::vec3 &value);
  glm::vec3 position() const;

private:
  std::vector<std::unique_ptr<Mesh>> meshes_;

  glm::vec3 position_{0.0f};

  Model(const Model &) = delete;
  void operator=(const Model &) = delete;
};
