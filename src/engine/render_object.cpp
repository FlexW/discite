#include "render_object.hpp"

namespace dc
{
RenderObject::RenderObject(std::shared_ptr<Model>    mesh,
                           std::shared_ptr<Material> material,
                           const glm::mat4 &         model_matrix)
    : mesh_{mesh},
      material_{material},
      model_matrix_{model_matrix},
      id_{generate_uuid()}
{
}

RenderObjectId RenderObject::id() const { return id_; }

Model *RenderObject::mesh() const { return mesh_.get(); }

Material *RenderObject::material() const { return material_.get(); }

} // namespace dc
