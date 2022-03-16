#pragma once

#include "material.hpp"
#include "mesh.hpp"
#include "uuid.hpp"

namespace dc
{
using RenderObjectId = Uuid;

class RenderObject
{
public:
  RenderObject(std::shared_ptr<Model>    mesh,
               std::shared_ptr<Material> material,
               const glm::mat4 &         model_matrix);

  RenderObjectId id() const;

  Model *   mesh() const;
  Material *material() const;

private:
  std::shared_ptr<Model>    mesh_{};
  std::shared_ptr<Material> material_{};
  glm::mat4                 model_matrix_;

  Uuid id_{};
};

} // namespace dc
